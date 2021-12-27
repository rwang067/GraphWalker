
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
    bid_t nmblocks; //number of in memory blocks
    vid_t **csrbuf;
    vid_t *csrbuf_pool;
    eid_t **beg_posbuf;
    bid_t cmblocks; //current number of in memory blocks
    bid_t *inMemIndex;
    int beg_posf, csrf;

    /* State */
    bid_t exec_block;
    bid_t nexec_blocks;
    
    /* Metrics */
    metrics &m;
    WalkManager *walk_manager;

    int cache_strategy = 1; //cache strategy, 0 realloc, 1 finest granularity, 2 adaptive memory pool 
        
    void print_config() {
        logstream(LOG_INFO) << "Engine configuration: " << std::endl;
        logstream(LOG_INFO) << " exec_threads = " << (int)exec_threads << std::endl;
        logstream(LOG_INFO) << " blocksize_kb = " << blocksize_kb << "kb" << std::endl;
        logstream(LOG_INFO) << " number of total blocks = " << nblocks << std::endl;
        logstream(LOG_INFO) << " number of in-memory blocks = " << nmblocks << std::endl;
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
    graphwalker_engine(std::string _base_filename, 
                       unsigned long long _blocksize_kb, 
                       bid_t _nblocks, 
                       bid_t _nmblocks, 
                       metrics &_m,
                       int _strategy=0) : 
                       base_filename(_base_filename), blocksize_kb(_blocksize_kb), nblocks(_nblocks), nmblocks(_nmblocks), m(_m) {
        exec_threads = get_option_int("execthreads", omp_get_max_threads());
        omp_set_num_threads(exec_threads);
        load_block_range(base_filename, blocksize_kb, blocks);
        logstream(LOG_INFO) << "block_range loaded!" << std::endl;
        nvertices = num_vertices();
        walk_manager = new WalkManager(m,nblocks,exec_threads,base_filename);
        logstream(LOG_INFO) << "walk_manager created!" << std::endl;

        set_cache_strategy(_strategy);
        logstream(LOG_INFO) << "cache strategy: " << (int)cache_strategy << "\n";
        if (cache_strategy == 2) { // the memory pool strategy
            csrbuf_pool = (vid_t*)malloc(nmblocks*blocksize_kb*1024);
        } else { // realloc or finest
            csrbuf = (vid_t**)malloc(nmblocks*sizeof(vid_t*));
            for(bid_t b = 0; b < nmblocks; b++){
                csrbuf[b] = (vid_t*)malloc(blocksize_kb*1024);
            }
        }

        logstream(LOG_INFO) << "csrbuf malloced!" << std::endl;
        beg_posbuf = (eid_t**)malloc(nmblocks*sizeof(eid_t*));

        inMemIndex = (bid_t*)malloc(nblocks*sizeof(bid_t));
        for(bid_t b = 0; b < nblocks; b++)  inMemIndex[b] = nmblocks;
        cmblocks = 0;

        std::string invlname = fidname( base_filename, 0 ); //only 1 file
        std::string beg_posname = invlname + ".beg_pos";
        std::string csrname = invlname + ".csr";
        beg_posf = open(beg_posname.c_str(),O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
        csrf = open(csrname.c_str(),O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
        if (csrf < 0 || beg_posf < 0) {
            logstream(LOG_FATAL) << "Could not load :" << csrname << " or " << beg_posname << ", error: " << strerror(errno) << std::endl;
        }
        assert(csrf > 0 && beg_posf > 0);

        _m.set("file", _base_filename);
        _m.set("engine", "default");
        _m.set("nblocks", (size_t)nblocks);

        print_config();
    }
        
    virtual ~graphwalker_engine() {
        delete walk_manager;
        
        if(inMemIndex != NULL) free(inMemIndex);
        if(blocks != NULL) free(blocks);

        if (cache_strategy == 2) {
            for (bid_t b = 0; b < cmblocks; b++) {
                if (beg_posbuf[b] != NULL) free(beg_posbuf[b]);
            }
            if (csrbuf_pool != NULL) free(csrbuf_pool);
        } else {
            for(bid_t b = 0; b < cmblocks; b++){
                if(beg_posbuf[b] != NULL)   free(beg_posbuf[b]);
                if(csrbuf[b] != NULL)   free(csrbuf[b]);
            }
        }
        if(beg_posbuf != NULL) free(beg_posbuf);
        if(csrbuf != NULL) free(csrbuf);

        close(beg_posf);  
        close(csrf);  
    }

    void set_cache_strategy(int _strategy) {
        if (_strategy >= 0 && _strategy <= 2)
            cache_strategy = _strategy;
        else
            logstream(LOG_FATAL) << "Don't support cache strategy: " << _strategy << "\n";
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

        /* read beg_pos file */
        if (cache_strategy == 1) { // finest granularity 
            *nverts = blocks[p+1] - blocks[p];
        } else {
            *nverts = blocks[p+nexec_blocks] - blocks[p];
        }

        beg_pos = (eid_t*) malloc((*nverts+1)*sizeof(eid_t));
        m.start_time("z__g_loadSubGraph_read_begpos");
        preada(beg_posf, beg_pos, (size_t)(*nverts+1)*sizeof(eid_t), (size_t)blocks[p]*sizeof(eid_t));        
        m.stop_time("z__g_loadSubGraph_read_begpos");
        /* read csr file */
        *nedges = beg_pos[*nverts] - beg_pos[0];
        if (*nedges*sizeof(vid_t) > blocksize_kb*1024){
            if (cache_strategy == 0 || cache_strategy == 1) {
                m.start_time("z__g_loadSubGraph_realloc_csr");
                logstream(LOG_WARNING) << "realloc_csr: nedges = " << *nedges << ", need size = " << ((*nedges)*sizeof(vid_t) >> 20) << "MB. " << std::endl;
                csr = (vid_t*)realloc(csr, (*nedges)*sizeof(vid_t) );
                m.stop_time("z__g_loadSubGraph_realloc_csr");     
            } else {
                logstream(LOG_FATAL) << "Current size :" << *nedges*sizeof(vid_t)/1024 << 
                                        " greater than Blocksize: " << blocksize_kb <<
                                        ". Use other cache strategies or increase block size!" << std::endl;
            }
        }
        m.start_time("z__g_loadSubGraph_read_csr");
        preada(csrf, csr, (*nedges)*sizeof(vid_t), beg_pos[0]*sizeof(vid_t));
        m.stop_time("z__g_loadSubGraph_read_csr");     

        m.stop_time("g_loadSubGraph");
    }

    void findSubGraph(bid_t p, eid_t * &beg_pos, vid_t * &csr, vid_t *nverts, eid_t *nedges){
        m.start_time("2_findSubGraph");
        vid_t *csr_buffer;
        bid_t blocks_offset = 1;
        if(inMemIndex[p] == nmblocks){//the block is not in memory
            bid_t swapin;
            if (cache_strategy == 2) {
                blocks_offset = nexec_blocks;
            }
            if (cmblocks+blocks_offset <= nmblocks) { // there are enough idle space in cache 
                swapin = cmblocks;
                cmblocks += blocks_offset;
            } else {
                bid_t minmwb;
                if (cache_strategy == 0 || cache_strategy == 1) {
                    minmwb = swapOut();
                } else {
                    minmwb = swapOut_mempool();
                }
                swapin = inMemIndex[minmwb];
                for (bid_t b = 0; b < blocks_offset && b < nblocks; b++) {
                    inMemIndex[minmwb+b] = nmblocks;
                }
                assert(swapin < nmblocks);
                if(beg_posbuf[swapin] != NULL) free(beg_posbuf[swapin]);
                    // munmap(beg_posbuf[swapin], sizeof(eid_t)*(blocks[minmwb+1] - blocks[minmwb] + 1));
            }
            if (cache_strategy == 2) {
                csr_buffer = csrbuf_pool + swapin * (blocksize_kb*1024/sizeof(vid_t));
                loadSubGraph(p, beg_posbuf[swapin], csr_buffer, nverts, nedges);
            } else {
                loadSubGraph(p, beg_posbuf[swapin], csrbuf[swapin], nverts, nedges);
            }
            for (bid_t b = 0; b < blocks_offset && b < nblocks; b++) {
                inMemIndex[p+b] = swapin + b;
            }
        }else{
        }
        beg_pos = beg_posbuf[ inMemIndex[p] ];
        if (cache_strategy == 2) {
            csr = csrbuf_pool + inMemIndex[p] * (blocksize_kb*1024/sizeof(vid_t));
        } else {
            csr = csrbuf[inMemIndex[p]];
        }
        // csr = csrbuf[ inMemIndex[p] ];
        m.stop_time("2_findSubGraph");
    }

    void loadBegpos(std::string bname, eid_t * &beg_pos, vid_t nverts, vid_t off = 0){

        beg_pos = (eid_t*)malloc((nverts+1)*sizeof(eid_t));
        std::string beg_posname = bname + ".beg_pos";
        int beg_posf = open(beg_posname.c_str(),O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
        if (beg_posf < 0) {
            logstream(LOG_FATAL) << "Could not load :" << beg_posname << ", error: " << strerror(errno) << std::endl;
        }
        assert(beg_posf > 0);

        /* read beg_pos file */
        preada(beg_posf, beg_pos, (size_t)(nverts+1)*sizeof(eid_t), (size_t)(off)*sizeof(eid_t));

        close(beg_posf);
    }

    void loadCSR(std::string bname, vid_t * &csr, eid_t nedges, eid_t off = 0){
        if(nedges <= 0) return;

        std::string csrname = bname + ".csr";
        int csrf = open(csrname.c_str(), O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
        if (csrf < 0) {
            logstream(LOG_FATAL) << "Could not load :" << csrname << ", error: " << strerror(errno) << std::endl;
        }
        assert(csrf > 0);

        /* read csr file */
        csr = (vid_t*)malloc(nedges*sizeof(vid_t));
        preada(csrf, csr, nedges*sizeof(vid_t), off*sizeof(vid_t));

        close(csrf); 
    }

    bid_t swapOut(){
        m.start_time("z_g_swapOut");
        wid_t minmw = 0xffffffff;
        bid_t minmwb = 0;
        for(bid_t b = 0; b < nblocks; b++){
            if(inMemIndex[b]<nmblocks && walk_manager->walknum[b] < minmw){
                minmw = walk_manager->walknum[b];
                minmwb = b;
            }
        }
        m.start_time("z_g_swapOut");
        return minmwb;
    }

    bid_t swapOut_mempool() { // for strategy 2, memory pool
        m.start_time("z_g_swapOutMemPool");
        wid_t minmw = 0xffffffff;
        bid_t minmwb = 0;
        for(bid_t p = 0; p < nblocks; p += nexec_blocks) {
            if(inMemIndex[p] < nmblocks){
                wid_t count = 0;
                for (bid_t b = 0; b < nexec_blocks && p+b < nblocks; b++) {
                    count += walk_manager->walknum[p+b];
                }
                if (count < minmw) {
                    minmw = count;
                    minmwb = p;
                }
            }
        }
        m.start_time("z_g_swapOutMemPool");
        return minmwb;
    }

    virtual size_t num_vertices() {
        return blocks[nblocks];
    }

    void exec_updates(RandomWalk &userprogram, wid_t nwalks, eid_t *&beg_pos, vid_t *&csr){ //, VertexDataType* vertex_value){
        // unsigned count = walk_manager->readblockWalks(exec_block);
        m.start_time("5_exec_updates");
        wid_t off = 0;
        bid_t blocks_offset = nexec_blocks;
        if (cache_strategy == 1) {
            blocks_offset = 1;
        }
        vid_t stv = blocks[exec_block];
        vid_t env = blocks[exec_block+blocks_offset];
        for(bid_t b = 0; b < blocks_offset; b++ ){
            wid_t nwalks_curb = walk_manager->walknum[exec_block+b];
            if(nwalks_curb < 100) omp_set_num_threads(1);
            #pragma omp parallel for schedule(static)
                for(wid_t i = 0; i < nwalks_curb; i++ ){
                    WalkDataType walk = walk_manager->curwalks[off+i];
                    userprogram.updateByWalk(walk, i, exec_block, stv, env, beg_pos, csr, *walk_manager );//, vertex_value);
                }
                off += nwalks_curb;
            }
            // for(wid_t i = 0; i < nwalks; i++ ){
            //     WalkDataType walk = walk_manager->curwalks[i];
            //     userprogram.updateByWalk(walk, i, exec_block, beg_pos, csr, *walk_manager );//, vertex_value);
            // }
        m.stop_time("5_exec_updates");
        // walk_manager->writeblockWalks(exec_block);
    }

    void fine_grained_updates(RandomWalk &userprogram, wid_t nwalks){ 
        // unsigned count = walk_manager->readblockWalks(exec_block);
        m.start_time("6_fine_grained_updates");
        // logstream(LOG_DEBUG) << "6_fine_grained_updates : " << nwalks << std::endl;
        eid_t *beg_pos = (eid_t*)malloc(2*sizeof(eid_t));
        eid_t edgesize = 100;
        vid_t *csr = (vid_t*)malloc(edgesize*sizeof(vid_t));
        wid_t off = 0;
        for(bid_t b = 0; b < nblocks; b++ ){
            wid_t nwalks_curb = walk_manager->walknum[b];
            for(wid_t i = 0; i < nwalks_curb; i++ ){
                WalkDataType walk = walk_manager->curwalks[off+i];
                vid_t curvertex = walk_manager->getCurrentId(walk) + blocks[b];
                std::string bname = fidname( base_filename, 0 ); //only 1 file
                loadBegpos(bname, beg_pos, 1, curvertex);
                eid_t nedges = beg_pos[1] - beg_pos[0];
                if(nedges > edgesize){
                    csr = (vid_t*)realloc(csr, nedges*sizeof(vid_t) );
                    edgesize = nedges;
                }
                loadCSR(bname, csr, nedges, beg_pos[0]);
                // logstream(LOG_DEBUG) << "updateByWalk : " << i << std::endl;
                userprogram.updateByWalk(walk, i, b, curvertex, curvertex+1, beg_pos, csr, *walk_manager );//, vertex_value);
            }
            off += nwalks_curb;
        }
        free(csr);
        free(beg_pos);
        m.stop_time("6_fine_grained_updates");
    }

    void run(RandomWalk &userprogram, float prob) {
        // srand((unsigned)time(NULL));
        m.start_time("0_startWalks");
        userprogram.startWalks(*walk_manager, nblocks, blocks, base_filename);
        m.stop_time("0_startWalks");
        
        gettimeofday(&start, NULL);
        m.start_time("00_runtime");

        vid_t nverts, *csr;
        eid_t nedges, *beg_pos;
        /*loadOnDemand -- block loop */
        int blockcount = 0;
        // while( walk_manager->walksum > 100000 ){
        while( walk_manager->walksum > 0 ){

            m.start_time("numExecBlocks");
            nexec_blocks = userprogram.numExecBlocks(*walk_manager, blocksize_kb);
            // nexec_blocks = 1;
            m.stop_time("numExecBlocks");
            m.start_time("1_chooseBlock");
            exec_block = walk_manager->chooseBlock(prob, nexec_blocks);
            if(exec_block + nexec_blocks > nblocks)
                nexec_blocks = nblocks - exec_block;
            m.stop_time("1_chooseBlock");
            findSubGraph(exec_block, beg_pos, csr, &nverts, &nedges);

            /*load walks info*/
            // walk_manager->loadWalkPool(exec_block);
            wid_t nwalks = walk_manager->getCurrentWalks(exec_block, nexec_blocks);
            if(nwalks <= 0){
                logstream(LOG_DEBUG) << runtime() << "s : blockcount: " << blockcount << std::endl;
                logstream(LOG_INFO) << "exec_block = " << exec_block << ", nexec_blocks = " << nexec_blocks << std::endl;
                logstream(LOG_INFO) << "nverts = " << nverts << ", nedges = " << nedges << std::endl;
                logstream(LOG_INFO) << "walksum = " << walk_manager->walksum << ", nwalks = " << nwalks << std::endl;
                for(bid_t b = 0; b < nexec_blocks; b++){
                    logstream(LOG_INFO) << "exec_block+b = " << exec_block+b << ", walk_manager->walknum[exec_block+b] = " << walk_manager->walknum[exec_block+b] << std::endl;
                }
                assert(false);
            }
            
            // if(blockcount % (nblocks/100+1)==1)
            if(blockcount % (1024*1024*1024/nedges+1) == 1)
            {
                logstream(LOG_DEBUG) << runtime() << "s : blockcount: " << blockcount << std::endl;
                logstream(LOG_INFO) << "exec_block = " << exec_block << ", nexec_blocks = " << nexec_blocks << std::endl;
                logstream(LOG_INFO) << "nverts = " << nverts << ", nedges = " << nedges << std::endl;
                logstream(LOG_INFO) << "walksum = " << walk_manager->walksum << ", nwalks = " << nwalks << std::endl;
            }
            
            exec_updates(userprogram, nwalks, beg_pos, csr);
            walk_manager->clearWalkNum(exec_block, nexec_blocks);
            walk_manager->updateWalkNum(exec_block, nexec_blocks);
            // logstream(LOG_INFO) << "After updateWalkNum : walksum = " << walk_manager->walksum << std::endl;
            // userprogram.compUtilization(0, walk_manager->walksum, nwalks, runtime());
            // userprogram.compUtilization(beg_pos[nverts] - beg_pos[0], walk_manager->walksum, nwalks, runtime());

            blockcount++;
        } // For block loop


        logstream(LOG_DEBUG) << runtime() << "s : begin fine-grained graph loading..., walksum = " << walk_manager->walksum << std::endl;
        //load all walks into memory
        while( walk_manager->walksum > 0 ){
            // logstream(LOG_INFO) << "Before, walksum = " << walk_manager->walksum << std::endl;
            wid_t nwalks = walk_manager->getCurrentWalks(0, nblocks);
            logstream(LOG_INFO) << "walksum = " << walk_manager->walksum << ", nwalks = " << nwalks << std::endl;
            fine_grained_updates(userprogram, nwalks);
            walk_manager->clearWalkNum(0, nblocks);
            walk_manager->updateWalkNum(0, 0);
            // logstream(LOG_INFO) << "After, walksum = " << walk_manager->walksum << std::endl;
        }


        m.stop_time("00_runtime");
    }
};

#endif