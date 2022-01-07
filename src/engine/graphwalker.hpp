
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

template <typename WalkDataType>
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
    
    /* In memory blocks */
    bid_t nmblocks; //number of in memory blocks
    int beg_posf, csrf;
    mem_pool* csrbuf_pool;
    block_index** blk_index;

    /* State */
    bid_t exec_block;
    bid_t nexec_blocks; // actual number of exec_blocks
    bid_t nexec_blocks1; // 2^n
    
    /* Metrics */
    metrics &m;
    WalkManager<WalkDataType> *walk_manager;

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
    graphwalker_engine(std::string _base_filename, 
                       unsigned long long _blocksize_kb, 
                       bid_t _nblocks, 
                       bid_t _nmblocks, 
                       metrics &_m) : 
                       base_filename(_base_filename), blocksize_kb(_blocksize_kb), nblocks(_nblocks), nmblocks(_nmblocks), m(_m) {
        exec_threads = get_option_int("execthreads", omp_get_max_threads());
        omp_set_num_threads(exec_threads);
        load_block_range(base_filename, blocksize_kb, blocks);
        logstream(LOG_INFO) << "block_range loaded!" << std::endl;
        nvertices = num_vertices();
        walk_manager = new WalkManager<WalkDataType>(m,nblocks,exec_threads,base_filename);
        logstream(LOG_INFO) << "walk_manager created!" << std::endl;

        csrbuf_pool = new mem_pool(nmblocks*blocksize_kb*1024);
        logstream(LOG_INFO) << "csrbuf_pool malloced!" << std::endl;
        // beg_posbuf = (eid_t**)malloc(nmblocks*sizeof(eid_t*));
        blk_index = (block_index**)malloc(nblocks*sizeof(block_index*));
        for(bid_t b = 0; b < nblocks; b++){
            blk_index[b] = NULL;
        }

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
        if(blocks != NULL) free(blocks);
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

        /* read beg_pos file */
        *nverts = blocks[p+nexec_blocks] - blocks[p];

        beg_pos = (eid_t*) malloc((*nverts+1)*sizeof(eid_t));
        m.start_time("z__g_loadSubGraph_read_begpos");
        preada(beg_posf, beg_pos, (size_t)(*nverts+1)*sizeof(eid_t), (size_t)blocks[p]*sizeof(eid_t));        
        m.stop_time("z__g_loadSubGraph_read_begpos");
        /* read csr file */
        *nedges = beg_pos[*nverts] - beg_pos[0];

        // csr = (vid_t*) malloc((*nedges)*sizeof(vid_t));
        size_t csr_size = nexec_blocks1 * blocksize_kb * 1024;
        assert((*nedges)*sizeof(vid_t) <= csr_size);
        if(csr_size > csrbuf_pool->size){
            bid_t b = mblockWithMinWalks();
            block_index* blk_index1 = blk_index[b];
            if(blk_index1->begpos) free(blk_index1->begpos);
            size_t free_size = blk_index1->csr_size;
            if(csr_size > free_size){
                logstream(LOG_DEBUG) << csr_size << " > " << free_size << std::endl;
            }
            assert(csr_size <= free_size);
            csrbuf_pool->reset((char*)blk_index1->csr, free_size);
            bid_t stb = blk_index1->stb;
            bid_t enb = blk_index1->enb;
            for(bid_t b1 = stb; b1 < enb; b1++){
                blk_index[b1] = NULL;
            }
            delete blk_index1;
        }
        csr = (vid_t*)csrbuf_pool->pool_alloc(csr_size);

        m.start_time("z__g_loadSubGraph_read_csr");
        preada(csrf, csr, (*nedges)*sizeof(vid_t), beg_pos[0]*sizeof(vid_t));
        m.stop_time("z__g_loadSubGraph_read_csr");     

        m.stop_time("g_loadSubGraph");
    }

    void findSubGraph(bid_t p, eid_t * &beg_pos, vid_t * &csr, vid_t *nverts, eid_t *nedges){
        m.start_time("2_findSubGraph");
        if(blk_index[p]==NULL){// the block is not in memory
            block_index* blk_index1 = new block_index();
            blk_index1->csr_size = nexec_blocks1 * blocksize_kb * 1024;
            blk_index1->stb = p;
            blk_index1->enb = p+nexec_blocks;
            loadSubGraph(p, blk_index1->begpos, blk_index1->csr, &(blk_index1->nverts), &(blk_index1->nedges));
            for(bid_t b = 0; b < nexec_blocks; b++){
                blk_index[p+b] = blk_index1;
            }
        }
        assert(blk_index[p]->csr);
        beg_pos = blk_index[p]->begpos;
        csr = blk_index[p]->csr;
        *nverts = blk_index[p]->nverts;
        *nedges = blk_index[p]->nedges;
        m.stop_time("2_findSubGraph");
    }

    bid_t mblockWithMinWalks(){
        m.start_time("z_g_swapOut");
        wid_t minmw = MAX_WALK_NUM;
        bid_t minmwp = 0;
        wid_t count = 0;
        for(bid_t p = 0; p < nblocks; p+=nexec_blocks1){
            if(blk_index[p]){
                count = 0;
                for(bid_t b = 0; b < nexec_blocks1 && p+b < nblocks; b++)
				    count += walk_manager->walknum[p+b];
                if(count < minmw){
                    minmw = count;
                    minmwp = p;
                }
            }
        }
		if(minmw >= MAX_WALK_NUM){
            logstream(LOG_DEBUG) << minmwp << " " << nexec_blocks1 << " " << minmw << " " << MAX_WALK_NUM << std::endl;
        }
		assert(minmw < MAX_WALK_NUM);
        m.start_time("z_g_swapOut");
        return minmwp;
    }

    virtual size_t num_vertices() {
        return blocks[nblocks];
    }

    void exec_updates1(RandomWalk<WalkDataType> &userprogram, wid_t nwalks, eid_t *&beg_pos, vid_t *&csr){ //, VertexDataType* vertex_value){
        // unsigned count = walk_manager->readblockWalks(exec_block);
        m.start_time("5_exec_updates");
        vid_t stv = blocks[exec_block];
        vid_t env = blocks[exec_block+nexec_blocks];
        // omp_set_num_threads(nwalks < 100? 1: exec_threads);
        #pragma omp parallel for schedule(static)
            for(wid_t i = 0; i < nwalks; i++ ){
                WalkDataType walk = walk_manager->curwalks[i];
                userprogram.updateByWalk(walk, i, exec_block, stv, env, beg_pos, csr, *walk_manager );//, vertex_value);
            }
        m.stop_time("5_exec_updates");
    }

    void exec_updates(RandomWalk<WalkDataType> &userprogram, wid_t nwalks, eid_t *&beg_pos, vid_t *&csr){ //, VertexDataType* vertex_value){
        // unsigned count = walk_manager->readblockWalks(exec_block);
        m.start_time("5_exec_updates");
        wid_t off = 0;
        vid_t stv = blocks[exec_block];
        vid_t env = blocks[exec_block+nexec_blocks];
        for(bid_t b = 0; b < nexec_blocks; b++ ){
            wid_t nwalks_curb = walk_manager->walknum[exec_block+b];
            if(nwalks_curb < 100) omp_set_num_threads(1);
            #pragma omp parallel for schedule(static)
                for(wid_t i = 0; i < nwalks_curb; i++ ){
                    WalkDataType walk = walk_manager->curwalks[off+i];
                    userprogram.updateByWalk(walk, i, exec_block, stv, env, beg_pos, csr, *walk_manager );//, vertex_value);
                }
                off += nwalks_curb;
            }
        m.stop_time("5_exec_updates");
    }

    void fine_grained_updates(RandomWalk<WalkDataType> &userprogram, wid_t nwalks){ 
        m.start_time("7_fine_grained_updates");
        eid_t *beg_pos = (eid_t*)malloc(2*sizeof(eid_t));
        eid_t edgesize = 100;
        vid_t *csr = (vid_t*)malloc(edgesize*sizeof(vid_t));
        wid_t off = 0;
        for(bid_t b = 0; b < nblocks; b++ ){
            wid_t nwalks_curb = walk_manager->walknum[b];
            for(wid_t i = 0; i < nwalks_curb; i++ ){
                WalkDataType walk = walk_manager->curwalks[off+i];
                vid_t curvertex = walk.currentId + blocks[b];
                std::string bname = fidname( base_filename, 0 ); //only 1 file
                // loadBegpos(bname, beg_pos, 1, curvertex);
                preada(beg_posf, beg_pos, 2*sizeof(eid_t), (size_t)(curvertex)*sizeof(eid_t));
                eid_t nedges = beg_pos[1] - beg_pos[0];
                if(nedges > edgesize){
                    csr = (vid_t*)realloc(csr, nedges*sizeof(vid_t) );
                    edgesize = nedges;
                }
                // loadCSR(bname, csr, nedges, beg_pos[0]);
                preada(csrf, csr, nedges*sizeof(vid_t), (size_t)(beg_pos[0])*sizeof(vid_t));
                userprogram.updateByWalk(walk, i, b, curvertex, curvertex+1, beg_pos, csr, *walk_manager );//, vertex_value);
            }
            off += nwalks_curb;
        }
        free(csr);
        free(beg_pos);
        m.stop_time("7_fine_grained_updates");
    }

    void run(RandomWalk<WalkDataType> &userprogram, float prob, wid_t fg_threshold = 100000) {
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
        do{
            m.start_time("1_chooseBlock");
            nexec_blocks1 = userprogram.numExecBlocks(*walk_manager, blocksize_kb);
            if(nexec_blocks1 > nmblocks || nmblocks == nblocks) nexec_blocks1 = nmblocks;
            exec_block = walk_manager->chooseBlock(prob, nexec_blocks1);
            nexec_blocks = nexec_blocks1;
            if(exec_block + nexec_blocks > nblocks)
                nexec_blocks = nblocks - exec_block;
            m.stop_time("1_chooseBlock");
            findSubGraph(exec_block, beg_pos, csr, &nverts, &nedges);

            /*load walks info*/
            // walk_manager->loadWalkPool(exec_block);
            wid_t nwalks = walk_manager->getCurrentWalks(exec_block, nexec_blocks);
            assert(nwalks > 0);
            
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
            // userprogram.compUtilization(beg_pos[nverts] - beg_pos[0], walk_manager->walksum, nwalks, runtime());

            blockcount++;
        }while( walk_manager->walksum > fg_threshold ); // For block loop


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