
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
    metrics &m;

    std::string base_filename;
    vid_t N;
    bid_t nblocks;
    std::vector<vid_t> blocks;
    
    size_t blocksize;
        
public:
        
    StaticGraph(metrics &_m, std::string _base_filename, vid_t _N, bid_t _nblocks, size_t _blocksize = 0) 
            : m(_m), base_filename(_base_filename), N(_N), nblocks(_nblocks), blocksize(_blocksize){
        // loadBlockRange();
    }

    // 当类有继承时，虚构函数必须为虚函数
    virtual ~StaticGraph() {
        m.set("nblocks", nblocks);
        blocks.clear();
    }

    size_t num_vertices() {
        return blocks[nblocks];
    }

    void loadBlockRange() {
        std::string blockrangefile = blockrangename(base_filename);
        std::ifstream brf(blockrangefile.c_str());
        
        if (!brf.good()) {
            logstream(LOG_ERROR) << "Could not load block range file: " << blockrangefile << std::endl;
        }
        assert(brf.good());
             
        blocks.resize(nblocks+1);
        vid_t en;
        for(bid_t i = 0; i < nblocks+1; i++) {
            assert(!brf.eof());
            brf >> en;
            blocks[i] = en;
        }
        for(bid_t i = nblocks-1; i < nblocks; i++) {
             logstream(LOG_INFO) << "Shard " << i << " : " << blocks[i] << " - " << blocks[i+1] << std::endl;
        }
        brf.close();
    }

    void writeBlockRange(){
        std::string blockrangefile = blockrangename(base_filename);
        std::ofstream brf(blockrangefile.c_str());
        for( bid_t p = 0; p <= nblocks; p++ ){
            brf << blocks[p] << std::endl;
        }
        brf.close();
    }

    bid_t getblock( vid_t v ){
        // for( bid_t p = 0; p < nblocks; p++ ){
        //     if( v < blocks[p+1] )
        //         return p;
        // }
        bid_t p = (bid_t)binarySearch(blocks.data(), v, 0, nblocks);
        assert(p < nblocks);
        return p;
    }
    
    void loadBegpos(std::string bname, eid_t * &beg_pos, vid_t nverts, vid_t off = 0){

        beg_pos = (eid_t*)malloc((nverts+1)*sizeof(eid_t));

        std::string beg_posname = bname + ".beg_pos";
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

    void loadSubGraph(bid_t p, eid_t * &beg_pos, vid_t * &csr, vid_t *nverts, eid_t *nedges){
        std::string bname = blockname( base_filename, p);
        *nverts = blocks[p+1] - blocks[p];
        loadBegpos(bname, beg_pos, *nverts);
        *nedges = beg_pos[*nverts] - beg_pos[0];
        loadCSR(bname, csr, *nedges);
    }

    void writeSubGraph(bid_t p, vid_t* csr, eid_t nedges, eid_t* beg_pos, vid_t nverts){
        std::string bname = blockname( base_filename, p);
        std::string beg_posname = bname + ".beg_pos";
        writefile(beg_posname, beg_pos, (size_t)(nverts+1)*sizeof(eid_t));
        std::string csrname = bname + ".csr";
        writefile(csrname, csr, (size_t)nedges*sizeof(vid_t));
    }

    std::vector<vid_t> getNeighbors(vid_t v){
        std::vector<vid_t> neighbors;

        bid_t p = getblock(v);
        std::string bname = blockname(base_filename, p);

        eid_t *beg_pos = (eid_t*)malloc(2*sizeof(eid_t));
        vid_t off = v - blocks[p];
        loadBegpos(bname, beg_pos, 1, off);

        eid_t nedges = beg_pos[1] - beg_pos[0];
        if(nedges > 0){
            // logstream(LOG_WARNING) << nedges << std::endl;
            vid_t *csr = (vid_t*)malloc(nedges*sizeof(vid_t));
            loadCSR(bname, csr, nedges, beg_pos[0]);
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