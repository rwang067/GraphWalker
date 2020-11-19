
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
    std::vector<vid_t> blocks;

    /* Metrics */
    metrics &m;
        
public:
        
    StaticGraph(std::string _base_filename, uint16_t _blocksize, bid_t _nblocks, metrics &_m) 
            : base_filename(_base_filename), blocksize(_blocksize), nblocks(_nblocks), m(_m){
        load_block_range(base_filename, blocksize, blocks);
        logstream(LOG_INFO) << "block_range loaded!" << std::endl;
        nvertices = num_vertices();
    }
        
    virtual ~StaticGraph() {
        // if(blocks != NULL) free(blocks);
        blocks.clear();
    }

    virtual size_t num_vertices() {
        return blocks[nblocks];
    }

    void load_block_range(std::string base_filename, uint16_t blocksize, std::vector<vid_t> &blocks, bool allowfail=false) {
        std::string blockrangefile = blockrangename(base_filename, blocksize);
        std::ifstream brf(blockrangefile.c_str());
        
        if (!brf.good()) {
            logstream(LOG_ERROR) << "Could not load block range file: " << blockrangefile << std::endl;
        }
        assert(brf.good());
        
        blocks.resize(nblocks+1);
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

        std::string beg_posname = blockname( base_filename, blocks[p] ) + ".beg_pos";
        FILE *tryf = fopen(beg_posname.c_str(), "r");
        if (tryf == NULL) { // Not found block beg_pos file
            // logstream(LOG_WARNING) << "Could not find the block beg_pos file : " << beg_posname << std::endl;
            memset(beg_pos, 0, (nverts+1)*sizeof(eid_t));
            return ;
        }
        fclose(tryf);

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
        if(nedges <= 0) return;

        std::string csrname = blockname( base_filename, blocks[p] ) + ".csr";
        int csrf = open(csrname.c_str(), O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
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

    void writeSubGraph(bid_t p, vid_t* csr, eid_t nedges, eid_t* beg_pos, vid_t nverts){
        std::string beg_posname = blockname( base_filename, blocks[p] ) + ".beg_pos";
        writefile(beg_posname, beg_pos, (size_t)(nverts+1)*sizeof(eid_t));
        std::string csrname = blockname( base_filename, blocks[p] ) + ".csr";
        writefile(csrname, csr, (size_t)nedges*sizeof(vid_t));
    }

    std::vector<vid_t> getNeighbors(vid_t v){
        std::vector<vid_t> neighbors;

        bid_t p = getblock(v);

        eid_t *beg_pos = (eid_t*)malloc(2*sizeof(eid_t));
        vid_t off = v - blocks[p];
        loadBegpos(p, beg_pos, 1, off);

        eid_t nedges = beg_pos[1] - beg_pos[0];
        if(nedges > 0){
            // logstream(LOG_WARNING) << nedges << std::endl;
            vid_t *csr = (vid_t*)malloc(nedges*sizeof(vid_t));
            loadCSR(p, csr, nedges, beg_pos[0]);
            for(eid_t i = 0; i < nedges; i++){
                neighbors.push_back(csr[i]);
            }
            free(csr);
        }
        free(beg_pos);

        return neighbors;
    }

   };

#endif