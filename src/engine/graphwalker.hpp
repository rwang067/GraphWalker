
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
    // unsigned membudget_mb;
    unsigned long long shardsize;  
    sid_t nshards;  
    vid_t nvertices;      
    tid_t exec_threads;
    vid_t* intervals;
    timeval start;
    
    /* State */
    sid_t exec_interval;
    
    /* Metrics */
    metrics &m;
    WalkManager *walk_manager;
        
    void print_config() {
        logstream(LOG_INFO) << "Engine configuration: " << std::endl;
        logstream(LOG_INFO) << " exec_threads = " << (int)exec_threads << std::endl;
        logstream(LOG_INFO) << " shardsize = " << shardsize << "kb" << std::endl;
        logstream(LOG_INFO) << " nshards = " << nshards << std::endl;
        // logstream(LOG_INFO) << " membudget_mb = " << membudget_mb << std::endl;
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
    graphwalker_engine(std::string _base_filename, unsigned long long _shardsize, sid_t _nshards, metrics &_m) : base_filename(_base_filename), shardsize(_shardsize), nshards(_nshards), m(_m) {
        // membudget_mb = get_option_int("membudget_mb", 1024);
        exec_threads = get_option_int("execthreads", omp_get_max_threads());
        omp_set_num_threads(exec_threads);
        load_vertex_intervals(base_filename, shardsize, intervals);
        nvertices = num_vertices();
        walk_manager = new WalkManager(m,nshards,exec_threads,base_filename);

        _m.set("file", _base_filename);
        _m.set("engine", "default");
        _m.set("nshards", (size_t)nshards);

        print_config();
    }
        
    virtual ~graphwalker_engine() {
    }

    void load_vertex_intervals(std::string base_filename, unsigned long long shardsize, vid_t * &intervals, bool allowfail=false) {
        std::string intervalsFilename = filename_intervals(base_filename, shardsize);
        std::ifstream intervalsF(intervalsFilename.c_str());
        
        if (!intervalsF.good()) {
            logstream(LOG_ERROR) << "Could not load intervals-file: " << intervalsFilename << std::endl;
        }
        assert(intervalsF.good());
        
        intervals = (vid_t*)malloc((nshards+1)*sizeof(vid_t));
        vid_t en;
        for(sid_t i=0; i < nshards+1; i++) {
            assert(!intervalsF.eof());
            intervalsF >> en;
            intervals[i] = en;
        }
        for(sid_t i=nshards-1; i < nshards; i++) {
             logstream(LOG_INFO) << "last shard: " << intervals[i] << " - " << intervals[i+1] << std::endl;
        }
        intervalsF.close();
    }

    void loadSubGraph(sid_t p, eid_t * &beg_pos, vid_t * &csr, vid_t *nverts, eid_t *nedges){
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

        /*output load graph info*/
        logstream(LOG_INFO) << "LoadSubGraph data end, with nverts = " << *nverts << ", " << "nedges = " << *nedges << std::endl;
        // logstream(LOG_INFO) << "beg_pos : "<< std::endl;
        // for(vid_t i = *nverts-10; i < *nverts; i++)
        //     logstream(LOG_INFO) << "beg_pos[" << i << "] = " << beg_pos[i] << ", "<< std::endl;
        // logstream(LOG_INFO) << "csr : "<< std::endl;
        // for(eid_t i = *nedges-10; i < *nedges; i++)
        //     logstream(LOG_INFO) << "csr[" << i << "] = " << csr[i] << ", "<< std::endl;
        m.stop_time("loadSubGraph");
    }

    virtual size_t num_vertices() {
        return intervals[nshards];
    }

    void exec_updates(RandomWalk &userprogram, eid_t *&beg_pos, vid_t *&csr){ //, VertexDataType* vertex_value){
        // unsigned count = walk_manager->readIntervalWalks(exec_interval);
        m.start_time("exec_updates");
        // logstream(LOG_INFO) << "exec_updates.." << std::endl;
        omp_set_num_threads(exec_threads);
        for(tid_t t = 0; t < exec_threads; t++){
            wid_t walkcount = walk_manager->pwalks[t][exec_interval].size();
            #pragma omp parallel for schedule(static)
                for(wid_t i = 0; i < walkcount; i++ ){
                    // logstream(LOG_INFO) << "exec_interval : " << exec_interval << " , walk : " << i << " --> threads." << omp_get_thread_num() << std::endl;
                    WalkDataType walk = walk_manager->pwalks[t][exec_interval][i];
                    userprogram.updateByWalk(walk, i, exec_interval, beg_pos, csr, *walk_manager );//, vertex_value);
                }
        }
        logstream(LOG_INFO) << "exec_updates end. Processsed walks with exec_threads = " << (int)exec_threads << std::endl;
        m.stop_time("exec_updates");
        // walk_manager->writeIntervalWalks(exec_interval);
    }

    void run(RandomWalk &userprogram, float prob) {
        gettimeofday(&start, NULL);
        // srand((unsigned)time(NULL));
        m.start_time("startWalks");
        userprogram.startWalks(*walk_manager, nshards, intervals);
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

            /*load graph info*/
            vid_t nverts, *csr;
            eid_t nedges, *beg_pos;
            loadSubGraph(exec_interval, beg_pos, csr, &nverts, &nedges);

            /*load walks info*/
            // walk_manager->loadWalkPool(exec_interval);

            userprogram.before_exec_interval(exec_interval, intervals[exec_interval], intervals[exec_interval+1], *walk_manager);
            exec_updates(userprogram, beg_pos, csr);
            userprogram.after_exec_interval(exec_interval, intervals[exec_interval], intervals[exec_interval+1], *walk_manager);
            // walk_manager->walknum[exec_interval] = 0;
            // walk_manager->writeWalkPools();

            free(beg_pos);
            free(csr);
            beg_pos = NULL;
            csr = NULL;

            m.stop_time("in_run_interval");
        } // For Interval loop
        m.stop_time("runtime");
    }
};

#endif