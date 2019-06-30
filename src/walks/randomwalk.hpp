#ifndef RANDOMWALK
#define RANDOMWALK

#include <string>
#include <fstream>
#include <time.h>

#include "walks/walk.hpp" 
#include "api/datatype.hpp"

/**
 * Type definitions. Remember to create suitable graph shards using the
 * Sharder-program.
 */
 
class RandomWalk {

public:
    bid_t nblocks;
    vid_t *blocks;
    wid_t nwalks;

public:

    //for SimRank
    virtual void startWalksbyApp( WalkManager &walk_manager, std::string base_filename){
        logstream(LOG_ERROR) << "No definition of function : startWalksbyApp!" << std::endl;
    }  

    //for msppr
    virtual void updateInfo(vid_t s, vid_t dstId, tid_t threadid, hid_t hop){
        logstream(LOG_ERROR) << "No definition of function : updateInfo!" << std::endl;
    }

    /**
     *  Walk update function.
     */
    virtual void updateByWalk(WalkDataType walk, wid_t walkid, bid_t exec_block, eid_t *&beg_pos, vid_t *&csr, WalkManager &walk_manager ){ //, VertexDataType* vertex_value){
        logstream(LOG_ERROR) << "No definition of function : updateByWalk!" << std::endl;
    }

    virtual void initializeRW( wid_t _nwalks, hid_t _nsteps) {
        logstream(LOG_DEBUG) << "No definition of function : initializeRW!" << std::endl;
    }
    
    /**
     * Called before an execution block is started.
     */
    virtual wid_t before_exec_block(bid_t exec_block, vid_t window_st, vid_t window_en, WalkManager &walk_manager) {
        logstream(LOG_DEBUG) << "No definition of function : before_exec_block!" << std::endl;
        return 0;
    }
    
    /**
     * Called after an execution block has finished.
     */
    // virtual void after_exec_block(unsigned exec_block, vid_t window_st, vid_t window_en, WalkManager &walk_manager, Vertex *&vertices) {
    virtual void after_exec_block(bid_t exec_block, vid_t window_st, vid_t window_en, WalkManager &walk_manager) {
        logstream(LOG_DEBUG) << "No definition of function : after_exec_block!" << std::endl;
    }

    virtual void compUtilization(eid_t total_edges){
        logstream(LOG_DEBUG) << "No definition of function : compUtilization!" << std::endl;
    }

    virtual void startWalks(WalkManager &walk_manager, bid_t _nblocks, vid_t* _blocks, std::string base_filename){
        nblocks = _nblocks;
        blocks = _blocks;
        startWalksbyApp(walk_manager, base_filename);
    }

    virtual unsigned getblock( vid_t v ){
        for( unsigned p = 0; p < nblocks; p++ ){
            if( v < blocks[p+1] )
                return p;
        }
        // logstream(LOG_DEBUG) << "v = " << v << ", blocks[nblocks] = " << blocks[nblocks] << std::endl;
        // assert(false);
        return nblocks;
    }
    
    /**
     * check if it has finished all walks
     */
    virtual bool hasFinishedWalk(WalkManager &walk_manager){
        wid_t remaining_walknum = walk_manager.walksum;
        // logstream(LOG_DEBUG) << "Walks remaining = " << remaining_walknum << std::endl;
        return ( remaining_walknum > 0 ); 
    }
    
};

#endif