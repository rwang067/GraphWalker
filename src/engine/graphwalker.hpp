
#ifndef DEF_GRAPHCHWALKER_ENGINE
#define DEF_GRAPHCHWALKER_ENGINE

#include "api/pthread_tools.hpp"
#include "walks/randomwalk.hpp"
#include "engine/memorygraph.hpp"

class GraphWalkerEngine{
public:   

    MemGraph *memgraph;

    WalkManager *walk_manager;
    bid_t exec_block;
    tid_t exec_threads;

    /* Metrics */
    metrics &m;
    timeval start;
        
    void print_config() {
        logstream(LOG_INFO) << "Engine configuration: " << std::endl;
        // logstream(LOG_INFO) << " blocksize = " << memgraph->graph->blocksize << "MB" << std::endl;
        logstream(LOG_INFO) << " number of total blocks = " << memgraph->graph->nblocks << std::endl;
        logstream(LOG_INFO) << " number of in-memory blocks = " << memgraph->nmblocks << std::endl;
        logstream(LOG_INFO) << " exec_threads = " << (int)exec_threads << std::endl;
    }

    double runtime() {
            timeval end;
            gettimeofday(&end, NULL);
            return end.tv_sec-start.tv_sec+ ((double)(end.tv_usec-start.tv_usec))/1.0E6;
        }
        
public:
        
    GraphWalkerEngine(std::string base_filename, uint16_t blocksize, bid_t nblocks, bid_t nmblocks, metrics &_m): m(_m){

        memgraph = new MemGraph(base_filename, blocksize,nblocks,nmblocks,m);

        exec_threads = get_option_int("execthreads", omp_get_max_threads());
        omp_set_num_threads(exec_threads);
        walk_manager = new WalkManager(m,nblocks,exec_threads,base_filename);
        logstream(LOG_INFO) << "walk_manager created!" << std::endl;

        print_config();

        _m.set("file", base_filename);
        _m.set("nblocks", (size_t)nblocks);
        _m.set("nmblocks", (size_t)nmblocks);
    }
        
    virtual ~GraphWalkerEngine() {
        delete walk_manager;
        delete memgraph;
    }

    void exec_updates(RandomWalk &userprogram, wid_t nwalks, eid_t *&beg_pos, vid_t *&csr){
        if(nwalks < 100) omp_set_num_threads(1);
        #pragma omp parallel for schedule(static)
            for(wid_t i = 0; i < nwalks; i++ ){
                WalkDataType walk = walk_manager->curwalks[i];
                userprogram.updateByWalk(walk, i, exec_block, beg_pos, csr, *walk_manager );
            }
    }

    void run(RandomWalk &userprogram, float prob) {
        // srand((unsigned)time(NULL));
        m.start_time("0_startWalks");
        userprogram.startWalks(*walk_manager, memgraph->graph->nblocks, &(memgraph->graph->blocks[0]), memgraph->graph->base_filename);
        m.stop_time("0_startWalks");
        
        gettimeofday(&start, NULL);
        m.start_time("runtime");

        vid_t nverts, *csr;
        eid_t nedges, *beg_pos;
        /*loadOnDemand -- block loop */
        int blockcount = 0;
        while( userprogram.hasFinishedWalk(walk_manager) ){
            blockcount++;
            m.start_time("1_chooseBlock");
            exec_block = walk_manager->chooseBlock(prob);
            m.stop_time("1_chooseBlock");

            m.start_time("2_findSubGraph");
            memgraph->findSubGraph(exec_block, beg_pos, csr, &nverts, &nedges, walk_manager);
            m.stop_time("2_findSubGraph");

            /*load walks info*/
            wid_t nwalks; 
            m.start_time("3_getCurrentWalks");
            nwalks = walk_manager->getCurrentWalks(exec_block);
            m.stop_time("3_getCurrentWalks");
            
            // if(blockcount % (1024*1024*1024/nedges+1) == 1)
            {
                logstream(LOG_INFO) << runtime() << "s : blockcount: " << blockcount << std::endl;
                logstream(LOG_INFO) << "nverts = " << nverts << ", nedges = " << nedges << std::endl;
                logstream(LOG_DEBUG) << "walksum = " << walk_manager->walksum << ", nwalks[" << exec_block << "] = " << nwalks << std::endl;
            }
            
            m.start_time("4_exec_updates");
            exec_updates(userprogram, nwalks, beg_pos, csr);
            m.stop_time("4_exec_updates");

            m.start_time("5_updateWalkNum");
            walk_manager->updateWalkNum(exec_block);
            m.stop_time("5_updateWalkNum");

        } // For block loop
        m.stop_time("runtime");
    }
};

#endif