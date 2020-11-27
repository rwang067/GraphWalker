
#ifndef DEF_MEMORY_GRAPH
#define DEF_MEMORY_GRAPH

#include "walks/randomwalk.hpp"
#include "engine/dynamicgraph.hpp"
#include "engine/staticgraph.hpp"

class MemGraph {
public:  
    
// #ifdef DYNAMIC_GRAPH
//     DynamicGraph *graph;
// #else
    StaticGraph *graph;
// #endif
    
    /* In memory blocks */
    bid_t nmblocks; //number of in memory blocks
    vid_t **csrbuf;
    eid_t **beg_posbuf;
    bid_t cmblocks; //current number of in memory blocks
    bid_t *inMemIndex;
        
public:
        
    MemGraph(std::string base_filename, uint16_t blocksize, bid_t nblocks, bid_t _nmblocks, metrics &m) 
            : nmblocks(_nmblocks) {

    // #ifdef DYNAMIC_GRAPH
    //     graph = new DynamicGraph(m, base_filename, nblocks);
    // #else
        graph = new StaticGraph(m, base_filename, nblocks, blocksize);
    // #endif

        csrbuf = (vid_t**)malloc(nmblocks*sizeof(vid_t*));
        for(bid_t b = 0; b < nmblocks; b++){
            csrbuf[b] = (vid_t*)malloc((size_t)blocksize*1024*1024);
        }

        eid_t max_nedges = (eid_t)blocksize * 1024 * 1024 / sizeof(vid_t); //max number of (vertices+edges) of a shard
        vid_t max_nvertices = (vid_t)max_nedges / 8; //max number of (vertices+edges) of a shard
        beg_posbuf = (eid_t**)malloc(nmblocks*sizeof(eid_t*));
        for(bid_t b = 0; b < nmblocks; b++){
            beg_posbuf[b] = (eid_t*)malloc(max_nvertices*sizeof(eid_t));
        }
        logstream(LOG_INFO) << "csrbuf and beg_posbuf malloced!" << std::endl;

        inMemIndex = (bid_t*)malloc(nblocks*sizeof(bid_t));
        for(bid_t b = 0; b < nblocks; b++)  inMemIndex[b] = nmblocks;
        cmblocks = 0;
    }
        
    virtual ~MemGraph() {
        if(inMemIndex != NULL) free(inMemIndex);

        for(bid_t b = 0; b < cmblocks; b++){
            if(beg_posbuf[b] != NULL)   free(beg_posbuf[b]);
            if(csrbuf[b] != NULL)   free(csrbuf[b]);
        }
        if(beg_posbuf != NULL) free(beg_posbuf);
        if(csrbuf != NULL) free(csrbuf);

        delete graph;
    }
    
    void findSubGraph(bid_t p, eid_t * &beg_pos, vid_t * &csr, vid_t *nverts, eid_t *nedges, WalkManager *walk_manager){
        if(inMemIndex[p] == nmblocks){//the block is not in memory
            bid_t swapin;
            if(cmblocks < nmblocks){
                swapin = cmblocks++;
            }else{
                bid_t minmwb = swapOut(walk_manager);
                swapin = inMemIndex[minmwb];
                inMemIndex[minmwb] = nmblocks;
                assert(swapin < nmblocks);
            }
            graph->loadSubGraph(p, beg_posbuf[swapin], csrbuf[swapin], nverts, nedges);
            inMemIndex[p] = swapin;
        }
        beg_pos = beg_posbuf[ inMemIndex[p] ];
        csr = csrbuf[ inMemIndex[p] ];
    }

    bid_t swapOut(WalkManager *walk_manager){
        wid_t minmw = 0xffffffff;
        bid_t minmwb = 0;
        for(bid_t b = 0; b < graph->nblocks; b++){
            if(inMemIndex[b]<nmblocks && walk_manager->walknum[b] < minmw){
                minmw = walk_manager->walknum[b];
                minmwb = b;
            }
        }
        return minmwb;
    }

   };

#endif