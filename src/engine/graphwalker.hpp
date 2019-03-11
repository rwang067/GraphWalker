
#ifndef DEF_GRAPHCHWALKER_ENGINE
#define DEF_GRAPHCHWALKER_ENGINE

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <omp.h>
#include <vector>
#include <map>
#include <sys/time.h>

#include "api/filename.hpp"
#include "api/io.hpp"
#include "logger/logger.hpp"
#include "metrics/metrics.hpp"
#include "api/pthread_tools.hpp"
#include "walks/randomwalk.hpp"

class graphwalker_engine {
public:     
    std::string base_filename;
    int shardsize;  
    int nshards;  
    int nvertices;      
    int membudget_mb;
    int exec_threads;
    std::vector<std::pair<vid_t, vid_t> > intervals;
    timeval start;
    
    /* State */
    int exec_interval;
    
    /* Metrics */
    metrics &m;
    WalkManager *walk_manager;
        
    void print_config() {
        logstream(LOG_INFO) << "Engine configuration: " << std::endl;
        logstream(LOG_INFO) << " exec_threads = " << exec_threads << std::endl;
        // logstream(LOG_INFO) << " load_threads = " << load_threads << std::endl;
        logstream(LOG_INFO) << " membudget_mb = " << membudget_mb << std::endl;
        // logstream(LOG_INFO) << " scheduler = " << use_selective_scheduling << std::endl;
    }

    double runtime() {
            timeval end;
            gettimeofday(&end, NULL);
            return end.tv_sec-start.tv_sec+ ((double)(end.tv_usec-start.tv_usec))/1.0E6;
        }
        
public:
        
    /**
     * Initialize GraphChi engine
     * @param base_filename prefix of the graph files
     * @param nshards number of shards
     * @param selective_scheduling if true, uses selective scheduling 
     */
    graphwalker_engine(std::string _base_filename, long long _shardsize, int _nshards, metrics &_m) : base_filename(_base_filename), shardsize(_shardsize), nshards(_nshards), m(_m) {
        // m.start_time("iomgr_init");
        // iomgr = new stripedio(m);
        // m.stop_time("iomgr_init");

        membudget_mb = get_option_int("membudget_mb", 1024);
        exec_threads = get_option_int("execthreads", omp_get_max_threads());
        logstream(LOG_INFO) << "Max available exec_threads = " << exec_threads << std::endl;
        omp_set_num_threads(exec_threads);

        load_vertex_intervals(base_filename, shardsize, intervals);
        nvertices = num_vertices();
        walk_manager = new WalkManager(m,nshards,exec_threads,base_filename);

        _m.set("file", _base_filename);
        _m.set("engine", "default");
        _m.set("nshards", (size_t)nshards);
    }
        
    virtual ~graphwalker_engine() {
    }

    void loadSubGraph(int p, eid_t * &beg_pos, vid_t * &csr, vid_t *nverts, eid_t *nedges){
        m.start_time("loadSubGraph");
        std::string invlname = intervalname( base_filename, p );
        std::string beg_posname = invlname + ".beg_pos";
        std::string csrname = invlname + ".csr";
        int beg_posf = open(beg_posname.c_str(),O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
        int csrf = open(csrname.c_str(),O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
        if (csrf < 0 || beg_posf < 0) {
            logstream(LOG_FATAL) << "Could not load :" << csrname << " or " << beg_posname << ", error: " << strerror(errno) << std::endl;
        }
        assert(csrf > 0 && beg_posf > 0);
        *nverts = readfull(beg_posf, &beg_pos) / sizeof(eid_t) - 1;
        *nedges = readfull(csrf, &csr) / sizeof(vid_t);

        // logstream(LOG_INFO) << "nverts = " << *nverts << ", " << "nedges = " << *nedges << std::endl;
        // logstream(LOG_INFO) << "beg_pos : "<< std::endl;
        // for(int i = 0; i < 10; i++)
        //     logstream(LOG_INFO) << beg_pos[i] << ", "<< std::endl;
        // logstream(LOG_INFO) << "csr : "<< std::endl;
        // for(int i = 0; i < 10; i++)
        //     logstream(LOG_INFO) << csr[i] << ", "<< std::endl;

        m.stop_time("loadSubGraph");
        // logstream(LOG_INFO) << "LoadSubGraph data end." << std::endl;
    }

    void load_vertex_intervals(std::string base_filename, long long shardsize, std::vector<std::pair<vid_t, vid_t> > & intervals, bool allowfail=false) {
        std::string intervalsFilename = filename_intervals(base_filename, shardsize);
        std::ifstream intervalsF(intervalsFilename.c_str());
        
        if (!intervalsF.good()) {
            logstream(LOG_ERROR) << "Could not load intervals-file: " << intervalsFilename << std::endl;
        }
        assert(intervalsF.good());
        
        intervals.clear();
        
        vid_t st=0, en;
        for(int i=0; i < nshards; i++) {
            assert(!intervalsF.eof());
            intervalsF >> en;
            intervals.push_back(std::pair<vid_t,vid_t>(st, en));
            st = en + 1;
        }
        // for(int i=0; i < nshards; i++) {
        for(int i=nshards-1; i < nshards; i++) {
             logstream(LOG_INFO) << "shard: " << intervals[i].first << " - " << intervals[i].second << std::endl;
        }
        intervalsF.close();
    }

    virtual size_t num_vertices() {
        return 1 + intervals[nshards - 1].second;
    }

    void exec_updates(RandomWalk &userprogram, eid_t *&beg_pos, vid_t *&csr){ //, VertexDataType* vertex_value){
        // unsigned count = walk_manager->readIntervalWalks(exec_interval);
        m.start_time("exec_updates");
        // logstream(LOG_INFO) << "exec_updates.." << std::endl;
        omp_set_num_threads(exec_threads);
        for(int t = 0; t < exec_threads; t++){
            unsigned count = walk_manager->pwalks[t][exec_interval].size();
            #pragma omp parallel for schedule(static)
                for(unsigned i = 0; i < count; i++ ){
                    // logstream(LOG_INFO) << "exec_interval : " << exec_interval << " , walk : " << i << " --> threads." << omp_get_thread_num() << std::endl;
                    WalkDataType walk = walk_manager->pwalks[t][exec_interval][i];
                    userprogram.updateByWalk(walk, i, exec_interval, beg_pos, csr, *walk_manager );//, vertex_value);
                }
        }
        m.stop_time("exec_updates");
        // walk_manager->writeIntervalWalks(exec_interval);
    }

    void run(RandomWalk &userprogram, float prob) {
        gettimeofday(&start, NULL);
        // srand((unsigned)time(NULL));
        m.start_time("startWalks");
        userprogram.startWalks(*walk_manager, nvertices, intervals);
        m.stop_time("startWalks");
        //initialnizeVertexData();

        m.start_time("runtime");
        /*loadOnDemand -- Interval loop */
        int numIntervals = 0;
        while( userprogram.hasFinishedWalk(*walk_manager) ){
            m.start_time("in_run_interval");
            numIntervals++;
            float cc = ((float)rand())/RAND_MAX;
            if( cc < prob ){
                // logstream(LOG_DEBUG) << "proc < 0.2 --> minstep, choose probability = " << cc << std::endl;
                exec_interval = walk_manager->intervalWithMinStep();
            }else{
                // logstream(LOG_DEBUG) << "proc > 0.2 --> maxwalk, choose probability = " << cc << std::endl;
                exec_interval =walk_manager->intervalWithMaxWalks();
            }
            if(numIntervals%10==1) logstream(LOG_DEBUG) << runtime() << "s : numIntervals: " << numIntervals << " : " << exec_interval << std::endl;
            //walk_manager->printWalksDistribution( exec_interval );
            /*load graph and walks info*/
            vid_t nverts, *csr;
            eid_t nedges, *beg_pos;
            loadSubGraph(exec_interval, beg_pos, csr, &nverts, &nedges);
            // unsigned count = walk_manager->readIntervalWalks(exec_interval);
            userprogram.before_exec_interval(exec_interval, intervals[exec_interval].first, intervals[exec_interval].second, *walk_manager);
            exec_updates(userprogram, beg_pos, csr);
            userprogram.after_exec_interval(exec_interval, intervals[exec_interval].first, intervals[exec_interval].second, *walk_manager);
            // userprogram.after_exec_interval(exec_interval, intervals[exec_interval].first, intervals[exec_interval].second, *walk_manager, vertices);

            m.start_time("freeSubGraph");
            free(beg_pos);
            free(csr);
            beg_pos = NULL;
            csr = NULL;
            m.stop_time("freeSubGraph");

            m.stop_time("in_run_interval");
        } // For Interval loop
        m.stop_time("runtime");
    }
};

#endif