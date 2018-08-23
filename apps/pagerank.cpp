
#define DYNAMICEDATA 1

#include <string>
#include <fstream>
#include <cmath>
#include <fstream>

#include "api/graphwalker_basic_includes.hpp"
#include "walks/randomwalkwithjump.hpp"
#include "util/toplist.hpp"

class PageRank : public RandomWalkwithJump{
private:
    unsigned N, R, L;
    unsigned *vertex_value;
    std::string vertex_value_file;
    vid_t cur_window_st;

public:
    void initializeApp( unsigned _N, unsigned _R, unsigned _L, float tail, std::string _basefilename ){
        N = _N;
        R = _R;
        L = _L;
        vertex_value_file = filename_vertex_data(_basefilename);
        vertex_value = (unsigned *)malloc(N*sizeof(unsigned));
        memset(vertex_value, 0, N*sizeof(unsigned));
        int f = open(vertex_value_file.c_str(), O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
        assert(f >= 0);
        pwritea(f, vertex_value, sizeof(unsigned)*N, 0);
        close(f);
        free(vertex_value);
        initializeRW( R, L, tail );
    }

    void startWalksbyApp( WalkManager &walk_manager  ){
        //muti threads to start walks
        int nthreads = get_option_int("execthreads", omp_get_max_threads());
        int cap = R/nshards/nthreads + 1;
        for( int p = 0; p < nshards; p++ ){
            for( int t = 0; t < nthreads; t++ )
                walk_manager.pwalks[t][p].reserve(cap);
            walk_manager.minstep[p] = 0;
        }
        srand((unsigned)time(NULL));
        omp_set_num_threads(nthreads);
        #pragma omp parallel for schedule(static)
            for( unsigned i = 0; i < N; i++ ){
                vid_t s = i;
                int p = getInterval(s);
                vid_t cur = s - intervals[p].first;
                WalkDataType walk = walk_manager.encode(i, cur, 0);
                for( unsigned i = 0; i < R; i++ ){
                    walk_manager.pwalks[omp_get_thread_num()][p].push_back(walk);
                }
            }

        walk_manager.freshIntervalWalks();
    }

    void updateInfo(vid_t dstId){
        vertex_value[dstId-cur_window_st]++; // #pragma omp critical
        // #pragma omp critical
        // {
        //     vertex_value[dstId-cur_window_st]++; // #pragma omp critical
        // }
    }

    /**
     * Called before an execution interval is started.
     */
    void before_exec_interval(vid_t window_st, vid_t window_en) {
        /*load vertex value*/
        // logstream(LOG_INFO) << "before_exec_interval : " << window_st << " " << window_en << std::endl;
        cur_window_st = window_st;
        unsigned  window_len =  window_en -  window_st + 1;
        vertex_value = (unsigned*)malloc(sizeof(unsigned)*window_len);
        int f = open(vertex_value_file.c_str(), O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
        assert(f >= 0);
        preada(f, vertex_value, sizeof(unsigned)*window_len, sizeof(unsigned)*window_st);
        close(f);
    }
    
    /**
     * Called after an execution interval has finished.
     */
    void after_exec_interval(vid_t window_st, vid_t window_en) {
         /*write back vertex back*/
        unsigned  window_len =  window_en -  window_st + 1;
        int f = open(vertex_value_file.c_str(), O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
        assert(f >= 0);
        pwritea(f, vertex_value, sizeof(unsigned)*window_len, sizeof(unsigned)*window_st);
        close(f);
        free(vertex_value);
    }

    void writeFile(){
        // compute the sum of counting
        vertex_value = (unsigned*)malloc(sizeof(unsigned)*N);
        int fv = open(vertex_value_file.c_str(), O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
        assert(fv >= 0);
        preada(fv, vertex_value, sizeof(unsigned)*N, sizeof(unsigned)*0);
        close(fv);
        unsigned sum = 0;
        for( unsigned i = 0; i < N; i++ )
            sum += vertex_value[i];
        logstream(LOG_INFO) << "sum : " << sum << std::endl;
        free(vertex_value);

        // conpute the counting probability
        unsigned maxwindow = 400000000;
        vid_t st = 0, len = 0;
        while( st < N ){
            len = N-st < maxwindow ? N-st : maxwindow;
            logstream(LOG_INFO) << " s , len : " << st << " " << len << std::endl;
            // len = min( maxwindow, N - st );
            vertex_value = (unsigned*)malloc(sizeof(unsigned)*len);
            fv = open(vertex_value_file.c_str(), O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
            assert(fv >= 0);
            preada(fv, vertex_value, sizeof(unsigned)*len, sizeof(unsigned)*st);
            close(fv);
            float *visit_prob = (float*)malloc(sizeof(float)*len);
            for( unsigned i = 0; i < len; i++ )
                visit_prob[i] = vertex_value[i] * 1.0 / sum;
            int fp = open(vertex_value_file.c_str(), O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
            assert(fp >= 0);
            pwritea(fp, visit_prob, sizeof(unsigned)*len, sizeof(unsigned)*st);
            close(fp);
            free(vertex_value);
            free(visit_prob);
            st += len;
        }
    }

    void computeError(int ntop){
        //read the vertex value
        float* visit_prob = (float*)malloc(sizeof(float)*N);
        int fv = open(vertex_value_file.c_str(), O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
        assert(fv >= 0);
        preada(fv, visit_prob, sizeof(float)*N, 0);
        close(fv);

        // read the accurate value and compute the error
        std::ifstream fin("/home/wang/Documents/dataset/LiveJournal/accurate_pr_top100.value");
        int vid ;
        float err=0, appv; //accurate pagerank value
        for(int i = 0; i < ntop; i++ ){
            fin >> vid >> appv;
            // logstream(LOG_INFO) << "vid appv vertex_value err: " << vid << " " << appv << " " << visit_prob[vid] << " " << fabs(visit_prob[vid]-appv)/appv << std::endl;
            err += fabs(visit_prob[vid]-appv)/appv;
        }
        free(visit_prob);
        err = err / ntop;
        logstream(LOG_DEBUG) << "Error : " << err << std::endl;

        std::ofstream errfile;
        errfile.open("/home/wang/Documents/dataset/LiveJournal/pr_top100.error", std::ofstream::app);
        errfile << err << "\n" ;
        errfile.close();
    }

};


int main(int argc, const char ** argv) {
    /* GraphChi initialization will read the command line
     arguments and the configuration file. */
    set_argc(argc, argv);
    
    /* Metrics object for keeping track of performance count_invectorers
     and other information. Currently required. */
    metrics m("randomwalk");
    
    /* Basic arguments for application */
    std::string filename = get_option_string("file", "../dataset/LiveJournal/soc-LiveJournal1.txt");  // Base filename
    int nvertices = get_option_int("nvertices", 4847571); // Number of vertices
    int R = get_option_int("R", 10); // Number of steps
    int L = get_option_int("L", 10); // Number of steps per walk
    float tail = get_option_float("tail", 0.05); // Ratio of stop long tail
    float prob = get_option_float("prob", 0.2); // prob of chose min step
    
    /* Detect the number of shards or preprocess an input to create them */
    int nshards = convert_if_notexists(filename, get_option_string("nshards", "auto"), nvertices, nvertices*R);

    /* Run */
    PageRank program;
    program.initializeApp( nvertices, R, L, tail, filename );
    graphwalker_engine engine(filename, nshards, m);
    engine.run(program, prob);
    
    program.writeFile();
    program.computeError(100);

    /* Report execution metrics */
    metrics_report(m);
    return 0;
}