
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
#include <sys/mman.h>
#include <asm/mman.h>

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
    unsigned long long blocksize_kb;  
    bid_t nblocks;  
    vid_t nvertices;      
    tid_t exec_threads;
    vid_t* blocks;
    timeval start;
    
    /* Ｉn memory blocks */
    bid_t ngblocks; //number of in memory blocks
    vid_t **csrbuf, *nverts;
    eid_t **beg_posbuf, *nedges;
    bid_t *inMemIndex;
    int beg_posf, csrf;

    /* State */
    bid_t exec_block;
    
    /* Metrics */
    metrics &m;
    WalkManager *walk_manager;
        
    void print_config() {
        logstream(LOG_INFO) << "Engine configuration: " << std::endl;
        logstream(LOG_INFO) << " exec_threads = " << (int)exec_threads << std::endl;
        logstream(LOG_INFO) << " blocksize_kb = " << blocksize_kb << "kb" << std::endl;
        logstream(LOG_INFO) << " number of total blocks = " << nblocks << std::endl;
        logstream(LOG_INFO) << " number of in-memory blocks = " << ngblocks << std::endl;
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
     * @param nblocks number of shards
     * @param selective_scheduling if true, uses selective scheduling 
     */
    graphwalker_engine(std::string _base_filename, unsigned long long _blocksize_kb, bid_t _nblocks, bid_t _ngblocks, metrics &_m) : base_filename(_base_filename), blocksize_kb(_blocksize_kb), nblocks(_nblocks), ngblocks(_ngblocks), m(_m) {
        // membudget_mb = get_option_int("membudget_mb", 1024);
        exec_threads = get_option_int("execthreads", omp_get_max_threads());
        omp_set_num_threads(exec_threads);
        load_block_range(base_filename, blocksize_kb, blocks);
        nvertices = num_vertices();
        walk_manager = new WalkManager(m,nblocks,exec_threads,base_filename);
        
        nverts = (vid_t*)malloc(ngblocks*sizeof(vid_t));
        nedges = (eid_t*)malloc(ngblocks*sizeof(eid_t));
        beg_posbuf = (eid_t**)malloc(ngblocks*sizeof(eid_t*));
        csrbuf = (vid_t**)malloc(ngblocks*sizeof(vid_t*));
        for(bid_t b = 0; b < ngblocks; b++){
            csrbuf[b] = (vid_t*)malloc(blocksize_kb*1024);
            // csrbuf[b] = (vid_t *)mmap(NULL, blocksize_kb*1024,
            //         PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS 
            //         //| MAP_HUGETLB | MAP_HUGE_2MB
            //         , 0, 0);
            // if(csrbuf[b] == MAP_FAILED){
            //     printf("%lld\n",b*blocksize_kb*1024);
            //     perror("csrbuf alloc mmap");
            //     exit(-1);
            // }
        }

        m.start_time("g_loadSubGraph_filename");
        std::string invlname = fidname( base_filename, 0 ); //only 1 file
        std::string beg_posname = invlname + ".beg_pos";
        std::string csrname = invlname + ".csr";
        m.stop_time("g_loadSubGraph_filename");
        m.start_time("g_loadSubGraph_open_begpos");
        beg_posf = open(beg_posname.c_str(),O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
        m.stop_time("g_loadSubGraph_open_begpos");
        m.start_time("g_loadSubGraph_open_csr");
        csrf = open(csrname.c_str(),O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
        m.stop_time("g_loadSubGraph_open_csr");
        m.start_time("g_loadSubGraph_if_open_success");
        if (csrf < 0 || beg_posf < 0) {
            logstream(LOG_FATAL) << "Could not load :" << csrname << " or " << beg_posname << ", error: " << strerror(errno) << std::endl;
        }
        assert(csrf > 0 && beg_posf > 0);
        m.stop_time("g_loadSubGraph_if_open_success");

        _m.set("file", _base_filename);
        _m.set("engine", "default");
        _m.set("nblocks", (size_t)nblocks);

        print_config();
    }
        
    virtual ~graphwalker_engine() {
        delete walk_manager;
        for(bid_t b = 0; b < ngblocks; b++){
            if(beg_posbuf[b] != NULL)   free(beg_posbuf[b]);
            if(csrbuf[b] != NULL)   free(csrbuf[b]);
                // munmap(csrbuf[b], blocksize_kb*1024);
        }
        free(nverts);
        free(nedges);
        free(beg_posbuf);
        free(csrbuf);
        free(blocks);

        close(beg_posf);  
        close(csrf);  
    }

    void load_block_range(std::string base_filename, unsigned long long blocksize_kb, vid_t * &blocks, bool allowfail=false) {
        std::string blockrangefile = blockrangename(base_filename, blocksize_kb);
        std::ifstream brf(blockrangefile.c_str());
        
        if (!brf.good()) {
            logstream(LOG_ERROR) << "Could not load block range file: " << blockrangefile << std::endl;
        }
        assert(brf.good());
        
        blocks = (vid_t*)malloc((nblocks+1)*sizeof(vid_t));
        vid_t en;
        for(bid_t i=0; i < nblocks+1; i++) {
            assert(!brf.eof());
            brf >> en;
            blocks[i] = en;
        }
        for(bid_t i=nblocks-1; i < nblocks; i++) {
             logstream(LOG_INFO) << "last shard: " << blocks[i] << " - " << blocks[i+1] << std::endl;
        }
        brf.close();
    }

    void loadSubGraph(bid_t p, eid_t * &beg_pos, vid_t * &csr, vid_t *nverts, eid_t *nedges){
        m.start_time("g_loadSubGraph");
        

        m.start_time("g_loadSubGraph_malloc_begpos");
        /* read beg_pos file */
        *nverts = blocks[p+1] - blocks[p];
        beg_pos = (eid_t*) malloc((*nverts+1)*sizeof(eid_t));
        m.stop_time("g_loadSubGraph_malloc_begpos");
        // beg_pos=(eid_t *)mmap(NULL,(size_t)(*nverts+1)*sizeof(eid_t),
        //         PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
        // if(beg_pos == MAP_FAILED)
        // {
        //     printf("%ld\n",(size_t)(*nverts+1)*sizeof(eid_t));
        //     perror("beg_pos alloc mmap");
        //     exit(-1);
        // }
        m.start_time("g_loadSubGraph_read_begpos");
        preada(beg_posf, beg_pos, (size_t)(*nverts+1)*sizeof(eid_t), (size_t)blocks[p]*sizeof(eid_t));        
        m.stop_time("g_loadSubGraph_read_begpos");
        /* read csr file */
        m.start_time("g_loadSubGraph_read_csr");
        *nedges = beg_pos[*nverts] - beg_pos[0];
        preada(csrf, csr, (*nedges)*sizeof(vid_t), beg_pos[0]*sizeof(vid_t));
        m.stop_time("g_loadSubGraph_read_csr");     

        /*output load graph info*/
        // logstream(LOG_INFO) << "LoadSubGraph data end, with nverts = " << *nverts << ", " << "nedges = " << *nedges << std::endl;
        
        // logstream(LOG_INFO) << "beg_pos : "<< std::endl;
        // for(vid_t i = *nverts-10; i < *nverts; i++)
        //     logstream(LOG_INFO) << "beg_pos[" << i << "] = " << beg_pos[i] << ", "<< std::endl;
        // logstream(LOG_INFO) << "csr : "<< std::endl;
        // for(eid_t i = *nedges-10; i < *nedges; i++)
        //     logstream(LOG_INFO) << "csr[" << i << "] = " << csr[i] << ", "<< std::endl;
        m.stop_time("g_loadSubGraph");
    }

    bid_t swapOut(){
        wid_t minmw = 0xffffffff;
        bid_t minmwb = 0;
        for(bid_t b = 0; b < nblocks; b++){
            if(inMemIndex[b]<ngblocks && walk_manager->walknum[b] < minmw){
                minmw = walk_manager->walknum[b];
                minmwb = b;
            }
        }
        // logstream(LOG_DEBUG) << "block " << minmwb << " is chosen to swap out!" << std::endl;
        // bid_t res = inMemIndex[minmwb];
        // inMemIndex[minmwb] = ngblocks;
        return minmwb;
    }

    virtual size_t num_vertices() {
        return blocks[nblocks];
    }

    void exec_updates(RandomWalk &userprogram, wid_t nwalks, eid_t *&beg_pos, vid_t *&csr){ //, VertexDataType* vertex_value){
        // unsigned count = walk_manager->readblockWalks(exec_block);
        m.start_time("exec_updates");
        #pragma omp parallel for schedule(static)
            for(wid_t i = 0; i < nwalks; i++ ){
                // logstream(LOG_INFO) << "exec_block : " << exec_block << " , walk : " << i << " --> threads." << omp_get_thread_num() << std::endl;
                WalkDataType walk = walk_manager->curwalks[i];
                userprogram.updateByWalk(walk, i, exec_block, beg_pos, csr, *walk_manager );//, vertex_value);
            }
        // logstream(LOG_INFO) << "exec_updates end. Processsed walks with exec_threads = " << (int)exec_threads << std::endl;
        m.stop_time("exec_updates");
        // walk_manager->writeblockWalks(exec_block);
    }

    void run(RandomWalk &userprogram, float prob) {
        gettimeofday(&start, NULL);
        m.start_time("__runtime__");
        // srand((unsigned)time(NULL));
        m.start_time("_startWalks");
        userprogram.startWalks(*walk_manager, nblocks, blocks, base_filename);
        m.stop_time("_startWalks");

        /*loadOnDemand -- block loop */
        int blockcount = 0;
        while( userprogram.hasFinishedWalk(*walk_manager) ){
            blockcount++;
            m.start_time("chooseBlock");
            // exec_block = walk_manager->chooseBlock(prob);
            bid_t* blocksgroup;
            bid_t nbbg = walk_manager->blocksgroupWithMaxWalks(ngblocks, blocksgroup);
            m.stop_time("chooseBlock");
            for(bid_t p = 0; p < nbbg; p++){
                loadSubGraph(blocksgroup[p], beg_posbuf[p], csrbuf[p], &nverts[p], &nedges[p]);
            }

            /*load walks info*/
            // walk_manager->loadWalkPool(exec_block);
            bool finishedGroup = false;
            while(!finishedGroup){
                finishedGroup = true;
                for(bid_t p = 0; p < nbbg; p++){
                    wid_t nwalks; 
                    exec_block = blocksgroup[p];
                    nwalks = walk_manager->getCurrentWalks(blocksgroup[p]);
                    
                    if(nwalks > 0){
                        
                    // if(blockcount % 100==1){
                        logstream(LOG_DEBUG) << runtime() << "s : blockcount: " << blockcount << " : " << exec_block << std::endl;
                        logstream(LOG_INFO) << "nverts = " << nverts << ", nedges = " << nedges << std::endl;
                        logstream(LOG_INFO) << "walksum = " << walk_manager->walksum << ", nwalks[p] = " << nwalks << std::endl;
                    // }
                    
                        finishedGroup = false;
                        exec_updates(userprogram, nwalks, beg_posbuf[p], csrbuf[p]);
                        walk_manager->updateWalkNum(blocksgroup[p]);
                    }
                }
            }

        } // For block loop
        m.stop_time("__runtime__");
    }
};

#endif