
#define KEEPWALKSINDISK 1

#include <string>
#include <fstream>
#include <cmath>
#include <fstream>

#include "api/graphwalker_basic_includes.hpp"
#include "walks/randomwalkwithrestart.hpp"
#include "util/toplist.hpp"
#include "util/comperror.hpp"

typedef unsigned VertexDataType;

class PersonalizedPageRank : public RandomWalkwithRestart{
public:
    vid_t source;
    unsigned nsources;
    unsigned N, R, L;
    VertexDataType **vertex_value;
    std::string basefilename;
    vid_t cur_window_st;

public:

    static std::string filename_vertex_data(std::string basefilename) {
        std::stringstream ss;
        ss << basefilename;
        ss << "_GraphWalker/" << sizeof(VertexDataType) << "B.vvalue";
        return ss.str();
    }

    void initializeApp( unsigned _N, vid_t _source, unsigned _R, unsigned _L, float tail, std::string _basefilename ){
        N = _N;
        source = _source;
        R = _R;
        L = _L;
        basefilename = _basefilename;
        #ifdef KEEPWALKSINDISK
            initialVertexValue<unsigned>(N, basefilename);
        #endif
        initializeRW( source, nsources*R, L, tail );
    }

    void startWalksbyApp( WalkManager &walk_manager  ){
        logstream(LOG_INFO) << "Start walks ! Total walk number = " << R*nsources << std::endl;
        unsigned nthreads = get_option_int("execthreads", omp_get_max_threads());
        unsigned cap = R/nthreads + 1;
        vid_t s = source;
        unsigned p = getInterval(s);
        for( unsigned t = 0; t < nthreads; t++ )
            walk_manager.pwalks[t][p].reserve(cap);
        walk_manager.minstep[p] = 0;
        walk_manager.walknum[p] += R;

        vid_t cur = s - intervals[p].first;
        WalkDataType walk = walk_manager.encode(s, cur, 0);
        omp_set_num_threads(nthreads);
        #pragma omp parallel for schedule(static)
            for( unsigned j = 0; j < R; j++ ){
                walk_manager.pwalks[omp_get_thread_num()][p].push_back(walk);
            }
        #ifdef KEEPWALKSINDISK
            walk_manager.freshIntervalWalks();
        #else
            vertex_value = new VertexDataType*[nthreads];
            for(unsigned i = 0; i < nthreads; i++){
                vertex_value[i] = new VertexDataType[N];
                memset(vertex_value[i], 0, N*sizeof(VertexDataType));
            }
        #endif
    }

    void updateInfo(vid_t sourId, vid_t dstId, unsigned threadid, unsigned hop){
        // if(hop >= L-10)
            vertex_value[threadid][dstId-cur_window_st]++; // #pragma omp critical
    }

    /**
     * Called before an execution interval is started.
     */
    void before_exec_interval(unsigned exec_interval, vid_t window_st, vid_t window_en, WalkManager &walk_manager) {
        #ifdef KEEPWALKSINDISK
            /*load walks*/
            walk_manager.readIntervalWalks(exec_interval);

            /*load vertex value*/
            cur_window_st = window_st;
            unsigned  window_len =  window_en -  window_st + 1;
            unsigned nthreads = get_option_int("execthreads", omp_get_max_threads());
            // logstream(LOG_DEBUG) << "nthreads = " << nthreads << std::endl;
            vertex_value = new VertexDataType*[nthreads];
            for(unsigned t = 0; t < nthreads; t++){
                vertex_value[t] = new VertexDataType[window_len];
                memset(vertex_value[t], 0, window_len*sizeof(VertexDataType));
            }
        #endif
    }
    
    /**
     * Called after an execution interval has finished.
     */
    void after_exec_interval(unsigned exec_interval, vid_t window_st, vid_t window_en, WalkManager &walk_manager) {
        walk_manager.walknum[exec_interval] = 0;
		walk_manager.minstep[exec_interval] = 0xfffffff;
        unsigned nthreads = get_option_int("execthreads", omp_get_max_threads());;
        for(unsigned t = 0; t < nthreads; t++)
            walk_manager.pwalks[t][exec_interval].clear();
        for( unsigned p = 0; p < nshards; p++){
            if(p == exec_interval ) continue;
            #ifndef KEEPWALKSINDISK
                walk_manager.walknum[p] = 0;
            #endif
			for(unsigned t=0;t<nthreads;t++){
				walk_manager.walknum[p] += walk_manager.pwalks[t][p].size();
            }
        }

        #ifdef KEEPWALKSINDISK
            /*write back walks*/
            walk_manager.writeIntervalWalks(exec_interval);

            /*write back vertex value*/
            unsigned  window_len =  window_en -  window_st + 1;
            VertexDataType *vertex_value_sum = (VertexDataType*)malloc(sizeof(VertexDataType)*window_len);
            unsigned f = open(filename_vertex_data(basefilename).c_str(), O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
            assert(f >= 0);
            preada(f, vertex_value_sum, sizeof(VertexDataType)*window_len, sizeof(VertexDataType)*window_st);
            close(f);
            for(unsigned i = 0; i < nthreads; i++){
                for(unsigned j = 0; j < window_len; j++){
                    vertex_value_sum[j] += vertex_value[i][j];
                }
            }
            f = open(filename_vertex_data(basefilename).c_str(), O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
            assert(f >= 0);
            pwritea(f, vertex_value_sum, sizeof(VertexDataType)*window_len, sizeof(VertexDataType)*window_st);
            close(f);
            free(vertex_value_sum);
            for(unsigned i = 0; i < nthreads; i++)
                free(vertex_value[i]);
            free(vertex_value);
        #endif
    }

};


int main(int argc, const char ** argv) {
    /* GraphChi initialization will read the command line
     arguments and the configuration file. */
    set_argc(argc, argv);
    
    /* Metrics object for keeping track of performance count_invectorers
     and other information. Currently required. */
    metrics m("personalizedpagerank");
    
    /* Basic arguments for application */
    std::string filename = get_option_string("file", "../DataSet/LiveJournal/soc-LiveJournal1.txt");  // Base filename
    unsigned nvertices = get_option_int("nvertices", 4847571); // Number of vertices
    unsigned source = get_option_int("source", 0); // vertex id of start source
    unsigned R = get_option_int("R", 2000); // Number of steps
    unsigned L = get_option_int("L", 10); // Number of steps per walk
    float tail = get_option_float("tail", 0); // Ratio of stop long tail
    float prob = get_option_float("prob", 0.2); // prob of chose min step
    long long shardsize = get_option_long("shardsize", 128); // Size of shard, represented in KB

    /* Detect the number of shards or preprocess an input to create them */
    unsigned nshards = convert_if_notexists(filename, shardsize);

    /* Run */
    PersonalizedPageRank program;
    program.initializeApp( nvertices, source, R, L, tail, filename );
    graphwalker_engine engine(filename, shardsize, nshards, m);
    engine.run(program, prob);

    #ifndef KEEPWALKSINDISK
        unsigned nthreads = get_option_int("execthreads", omp_get_max_threads());;
        for(unsigned t = 1; t < nthreads; t++){
            for(unsigned i = 0; i < nvertices; i++ ){
                program.vertex_value[0][i] += program.vertex_value[t][i];
            }
        }
        unsigned f = open(filename_vertex_data(filename).c_str(), O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
        assert(f >= 0);
        pwritea(f, program.vertex_value[0], sizeof(VertexDataType)*nvertices, 0);
        close(f);
        free(program.vertex_value[0]);
    #endif
    
    // computeError<unsigned>(nvertices, filename, 100, "ppr");
    
    // /* List top 20 */
    // unsigned ntop = 20;
    // std::vector< vertex_value<VertexDataType> > top = get_top_vertices<VertexDataType>(filename, ntop);
    // std::cout << "Print top 20 vertices: " << std::endl;
    // for(unsigned i=0; i < (unsigned) top.size(); i++) {
    //     std::cout << (i+1) << ". " << top[i].vertex << "\t" << top[i].value << std::endl;
    // }
    // // read the accurate value
    //     float *ppv = (float*)malloc(nvertices*sizeof(float));
    //     std::string PPV_file = "/home/wang/Documents/dataset/LiveJournal/PPR0.vout";
    //     unsigned f1 = open(PPV_file.c_str(), O_RDONLY, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
    //     if (f1 < 0) {
    //         logstream(LOG_ERROR) << "Could not open " << PPV_file << " error: " << strerror(errno) << std::endl;
    //     }
    //     assert(f1 >= 0);
    //     preada(f1, ppv, nvertices*sizeof(float), 0);
    //     close(f1);
    //     //compute the error
    //     float err = 0;
    //     for( unsigned i = 0; i < ntop; i++ ){
    //         err += fabs(top[i].value - ppv[top[i].vertex])/ppv[top[i].vertex];//(ppv[i]-visit_prob[i])*(ppv[i]-visit_prob[i]);
    //     }
    //     err = err / ntop;
    //     std::cout << "Error : " << err << std::endl;

    //     std::ofstream errfile;
    //     errfile.open("/home/wang/Documents/graph processing system/dataset/LiveJournal1/ppv0.error", std::ofstream::app);
    //     errfile << err << "\n" ;
    //     errfile.close();
    //     free(ppv);


    //std::cout << "average degree : " << program.count << " " << program.degree*1.0/program.count << std::endl;

    /* Report execution metrics */
    metrics_report(m);
    return 0;
}