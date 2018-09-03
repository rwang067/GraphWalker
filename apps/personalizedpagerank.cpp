
#define DYNAMICEDATA 1

#include <string>
#include <fstream>
#include <cmath>
#include <fstream>

#include "api/graphwalker_basic_includes.hpp"
#include "walks/randomwalkwithrestart.hpp"
#include "util/toplist.hpp"
#include "util/comperror.hpp"
const bool semi_external = true;

class PersonalizedPageRank : public RandomWalkwithRestart{
public:
    vid_t source;
    unsigned nsources;
    unsigned N, R, L;
    VertexDataType **vertex_value;
    std::string basefilename;
    vid_t cur_window_st;

public:
    void initializeApp( unsigned _N, vid_t _source, unsigned _nsources, unsigned _R, unsigned _L, float tail, std::string _basefilename ){
        N = _N;
        source = _source;
        nsources = _nsources;
        R = _R;
        L = _L;
        basefilename = _basefilename;
        initialVertexValue<unsigned>(N, basefilename);
        initializeRW( source, nsources*R, L, tail );
    }

    void startWalksbyApp( WalkManager &walk_manager  ){
        logstream(LOG_INFO) << "Start walks ! Total walk number = " << R*nsources << std::endl;
        int nthreads = get_option_int("execthreads", omp_get_max_threads());
        int cap = R/nthreads + 1;
        vid_t s = source;
        int p = getInterval(s);
        for( int t = 0; t < nthreads; t++ )
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
        if(semi_external){ 
            vertex_value = new VertexDataType*[nthreads];
            for(int i = 0; i < nthreads; i++){
                vertex_value[i] = new VertexDataType[N];
                memset(vertex_value[i], 0, N*sizeof(VertexDataType));
            }
            return ;
        }
        walk_manager.freshIntervalWalks();
    }

    void updateInfo(vid_t dstId, unsigned threadid, unsigned hop){
        // if(hop >= L-10)
            vertex_value[threadid][dstId-cur_window_st]++; // #pragma omp critical
    }

    /**
     * Called before an execution interval is started.
     */
    void before_exec_interval(int exec_interval, vid_t window_st, vid_t window_en, WalkManager &walk_manager) {
        if(semi_external) return ;
        /*load walks*/
        walk_manager.readIntervalWalks(exec_interval);

        /*load vertex value*/
        cur_window_st = window_st;
        unsigned  window_len =  window_en -  window_st + 1;
        unsigned nthreads = get_option_int("execthreads");
        vertex_value = new VertexDataType*[nthreads];
		for(unsigned t = 0; t < nthreads; t++){
			vertex_value[t] = new VertexDataType[window_len];
            memset(vertex_value[t], 0, window_len*sizeof(VertexDataType));
        }
    }
    
    /**
     * Called after an execution interval has finished.
     */
    void after_exec_interval(int exec_interval, vid_t window_st, vid_t window_en, WalkManager &walk_manager) {
        walk_manager.walknum[exec_interval] = 0;
		walk_manager.minstep[exec_interval] = 0xfffffff;
        unsigned nthreads = get_option_int("execthreads");
        for( int p = 0; p < nshards; p++){
            if(p == exec_interval ) continue;
            if(semi_external) walk_manager.walknum[p] = 0;
			for(unsigned t=0;t<nthreads;t++){
				walk_manager.walknum[p] += walk_manager.pwalks[t][p].size();
            }
        }

        if(semi_external){ 
            for(unsigned t = 0; t < nthreads; t++)
                walk_manager.pwalks[t][exec_interval].clear();
            return ;
        }
         /*write back walks*/
        walk_manager.writeIntervalWalks(exec_interval);

         /*write back vertex value*/
        unsigned  window_len =  window_en -  window_st + 1;
        VertexDataType *vertex_value_sum = (VertexDataType*)malloc(sizeof(VertexDataType)*window_len);
        int f = open(filename_vertex_data(basefilename).c_str(), O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
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
        free(vertex_value);
        for(unsigned i = 0; i < nthreads; i++)
            free(vertex_value[i]);
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
    int nvertices = get_option_int("nvertices", 4847571); // Number of vertices
    long long nedges = get_option_long("nedges", 68993773); // Number of vertices
    int nshards = get_option_int("nshards", 0); // Number of vertices
    int source = get_option_int("source", 0); // vertex id of start source
    int nsources = get_option_int("nsources", 1); // Number of sources
    int R = get_option_int("R", 10000); // Number of steps
    int L = get_option_int("L", 10); // Number of steps per walk
    float tail = get_option_float("tail", 0); // Ratio of stop long tail
    float prob = get_option_float("prob", 0.2); // prob of chose min step

    /* Detect the number of shards or preprocess an input to create them */
    nshards = convert_if_notexists(filename, get_option_string("nshards", "auto"), nvertices, nedges, nsources*R, nshards);

    /* Run */
    PersonalizedPageRank program;
    program.initializeApp( nvertices, source, nsources, R, L, tail, filename );
    graphwalker_engine engine(filename, nshards, m);
    engine.run(program, prob);

    if(semi_external){
        unsigned nthreads = get_option_int("execthreads");
        for(unsigned t = 1; t < nthreads; t++){
            for(int i = 0; i < nvertices; i++ ){
                program.vertex_value[0][i] += program.vertex_value[t][i];
            }
        }
        int f = open(filename_vertex_data(filename).c_str(), O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
        assert(f >= 0);
        pwritea(f, program.vertex_value[0], sizeof(VertexDataType)*nvertices, 0);
        close(f);
        free(program.vertex_value[0]);
    }
    
    computeError<unsigned>(nvertices, filename, 100, "ppr");
    
    // /* List top 20 */
    // int ntop = 20;
    // std::vector< vertex_value<VertexDataType> > top = get_top_vertices<VertexDataType>(filename, ntop);
    // std::cout << "Print top 20 vertices: " << std::endl;
    // for(int i=0; i < (int) top.size(); i++) {
    //     std::cout << (i+1) << ". " << top[i].vertex << "\t" << top[i].value << std::endl;
    // }
    // // read the accurate value
    //     float *ppv = (float*)malloc(nvertices*sizeof(float));
    //     std::string PPV_file = "/home/wang/Documents/dataset/LiveJournal/PPR0.vout";
    //     int f1 = open(PPV_file.c_str(), O_RDONLY, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
    //     if (f1 < 0) {
    //         logstream(LOG_ERROR) << "Could not open " << PPV_file << " error: " << strerror(errno) << std::endl;
    //     }
    //     assert(f1 >= 0);
    //     preada(f1, ppv, nvertices*sizeof(float), 0);
    //     close(f1);
    //     //compute the error
    //     float err = 0;
    //     for( int i = 0; i < ntop; i++ ){
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