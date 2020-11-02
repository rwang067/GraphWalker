
#ifndef DEF_STATIC_GRAPH
#define DEF_STATIC_GRAPH

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

class StaticGraph {
public:     
    std::string base_filename;
    vid_t nvertices;
    uint16_t blocksize;
    bid_t nblocks;
    vid_t* blocks;
        
public:
        
    StaticGraph(std::string _base_filename, uint16_t _blocksize, bid_t _nblocks, bid_t _nmblocks) 
            : base_filename(_base_filename), blocksize(_blocksize), nblocks(_nblocks){
        load_block_range(base_filename, blocksize, blocks);
        logstream(LOG_INFO) << "block_range loaded!" << std::endl;
        nvertices = num_vertices();
    }
        
    virtual ~StaticGraph() {
        if(blocks != NULL) free(blocks);
    }

    virtual size_t num_vertices() {
        return blocks[nblocks];
    }

    void load_block_range(std::string base_filename, uint16_t blocksize, vid_t * &blocks, bool allowfail=false) {
        std::string blockrangefile = blockrangename(base_filename, blocksize);
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

    bid_t getblock( vid_t v ){
        for( bid_t p = 0; p < nblocks; p++ ){
            if( v < blocks[p+1] )
                return p;
        }
        return nblocks;
    }
    
    void loadBegpos(bid_t p, eid_t * &beg_pos, vid_t nverts, vid_t off = 0){
        std::string beg_posname = fidname( base_filename, p ) + ".beg_pos";
        int beg_posf = open(beg_posname.c_str(),O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
        if (beg_posf < 0) {
            logstream(LOG_FATAL) << "Could not load :" << beg_posname << ", error: " << strerror(errno) << std::endl;
        }
        assert(beg_posf > 0);

        /* read beg_pos file */
        preada(beg_posf, beg_pos, (size_t)(nverts+1)*sizeof(eid_t), (size_t)(off)*sizeof(eid_t));

        close(beg_posf);  
    }

    void loadCSR(bid_t p, vid_t * &csr, eid_t nedges, eid_t off = 0){
        std::string csrname = fidname( base_filename, p ) + ".csr";
        int csrf = open(csrname.c_str(),O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
        if (csrf < 0) {
            logstream(LOG_FATAL) << "Could not load :" << csrname << ", error: " << strerror(errno) << std::endl;
        }
        assert(csrf > 0);

        /* read csr file */
        if(nedges*sizeof(vid_t) > (size_t)blocksize*1024*1024){
            csr = (vid_t*)realloc(csr, (nedges)*sizeof(vid_t) );
        }
        preada(csrf, csr, nedges*sizeof(vid_t), off*sizeof(vid_t));

        close(csrf); 
    }

    void loadSubGraph(bid_t p, eid_t * &beg_pos, vid_t * &csr, vid_t *nverts, eid_t *nedges){
        *nverts = blocks[p+1] - blocks[p];
        loadBegpos(p, beg_pos, *nverts);
        *nedges = beg_pos[*nverts] - beg_pos[0];
        loadCSR(p, csr, *nedges);
    }

    std::vector<vid_t> getNeighbors(vid_t v){
        bid_t p = getblock(v);
        vid_t off = v - blocks[p];
        eid_t nedges, *beg_pos = new eid_t[2];
        vid_t *csr = new vid_t[1000];

        loadBegpos(p, beg_pos, 1, off);
        nedges = beg_pos[1] - beg_pos[0];
        loadCSR(p, csr, nedges, beg_pos[0]);

        std::vector<vid_t> neighbors;
        for(eid_t i = 0; i < nedges; i++){
            neighbors.push_back(csr[i]);
        }

        delete csr;
        delete beg_pos;

        return neighbors;
    }

   };

#endif