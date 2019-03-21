
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

#include "cuda/exec_update.hpp"

class graphwalker_engine {
public:     
    std::string base_filename;
    unsigned membudget_mb;
    long long shardsize;  
    sid_t nshards;  
    vid_t nvertices;      
    int exec_threads;
    vid_t* intervals;
    timeval start;
    
    /* State */
    sid_t exec_interval;
    
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
    graphwalker_engine(std::string _base_filename, long long _shardsize, sid_t _nshards, metrics &_m) : base_filename(_base_filename), shardsize(_shardsize), nshards(_nshards), m(_m) {
        // m.start_time("iomgr_init");
        // iomgr = new stripedio(m);
        // m.stop_time("iomgr_init");

        membudget_mb = get_option_int("membudget_mb", 1024);
        exec_threads = get_option_int("execthreads", omp_get_max_threads());
        logstream(LOG_INFO) << "Max available exec_threads = " << exec_threads << std::endl;
        omp_set_num_threads(exec_threads);
        walk_manager = new WalkManager(m,nshards,base_filename);

        loadIntervals();

        _m.set("file", _base_filename);
        _m.set("engine", "default");
        _m.set("nshards", nshards);
    }
        
    virtual ~graphwalker_engine() {
    }

    void loadIntervals(){
        // load_vertex_intervals(base_filename, shardsize, intervals);
        std::string intervalsname = filename_intervals(base_filename, shardsize);
        int invlf = open(intervalsname.c_str(),O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
        if (invlf < 0) {
            logstream(LOG_FATAL) << "Could not load :" << intervalsname << std::endl;
        }
        assert(invlf > 0);
        size_t sz = readfull(invlf, &intervals);
        if( (sid_t)(sz/sizeof(vid_t) - 1) != nshards){
            logstream(LOG_DEBUG) << "(sid_t)(sz/sizeof(vid_t) - 1) = " << (sid_t)(sz/sizeof(vid_t) - 1) << ", nshards = " << nshards << std::endl;
            assert(false);
        }
        nvertices = intervals[nshards];
        for(sid_t p = 0; p < nshards; p++){
            logstream(LOG_INFO) << "Interval " << p << " : [ " << intervals[p] << " , " << intervals[p+1] << " ] " << std::endl;
        }
    }

    void loadSubGraph(sid_t p, eid_t * &beg_pos, vid_t * &csr, vid_t *nverts, eid_t *nedges){
        m.start_time("loadSubGraph");
        std::string invlname = prefixname( base_filename, p );
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

    void exec_updates_cpu(eid_t *&beg_pos, vid_t *&csr, sid_t exec_interval, vid_t* intervals, WalkDataType* walks, WalkDataType **&pwalks, vid_t nverts, eid_t nedges, wid_t nwalks, sid_t nshards){
        m.start_time("exec_updates in CPU");
        // logstream(LOG_INFO) << "exec_updates.." << std::endl;
        omp_set_num_threads(exec_threads);
        for(int t = 0; t < exec_threads; t++){
            #pragma omp parallel for schedule(static)
                for(unsigned i = 0; i < nwalks; i++ ){
                    ;//WalkDataType walk = walks[i];
                    //updateByWalk(walk, i, exec_interval, vertices, *walk_manager );//, vertex_value);
                }
        }
        m.stop_time("exec_updates in CPU");
    }

    void run(RandomWalk &userprogram, float prob) {
        gettimeofday(&start, NULL);
        m.start_time("startWalks");
        userprogram.startWalks(*walk_manager, nvertices, nshards, intervals);
        m.stop_time("startWalks");

        m.start_time("runtime");
        /*loadOnDemand -- Interval loop */
        int numIntervals = 0;
        while( walk_manager->walkSum() > 0 ){
            numIntervals++;
            // float cc = ((float)rand())/RAND_MAX;
            // if( cc < prob ){
            //     // logstream(LOG_DEBUG) << "proc < 0.2 --> minstep, choose probability = " << cc << std::endl;
            //     exec_interval = walk_manager->intervalWithMinStep();
            // }else{
            //     // logstream(LOG_DEBUG) << "proc > 0.2 --> maxwalk, choose probability = " << cc << std::endl;
                exec_interval =walk_manager->intervalWithMaxWalks();
            // }
            if(numIntervals%10==0) logstream(LOG_DEBUG) << runtime() << "s : numIntervals: " << numIntervals << " : " << exec_interval << std::endl;
            //walk_manager->printWalksDistribution( exec_interval );

            /*load graph info*/
            vid_t nverts, *csr;
            eid_t nedges, *beg_pos;
            loadSubGraph(exec_interval, beg_pos, csr, &nverts, &nedges);

            /*load walks info*/
            walk_manager->loadWalkPool(exec_interval);
            
            exec_updates(m, beg_pos, csr, exec_interval, intervals, walk_manager->walks, walk_manager->pwalks, walk_manager->pnwalks, nverts, nedges, walk_manager->nwalks, nshards);
            
            walk_manager->walknum[exec_interval] = 0;
            walk_manager->writeWalkPools();

            free(beg_pos);
            free(csr);

        } // For Interval loop
        m.stop_time("runtime");
    }
};

#endif