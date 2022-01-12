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

class WalkDataType{
public:
    uint64_t sourceId:24;
    uint64_t currentId:26;
    uint64_t hop:14;
    WalkDataType(vid_t _sourceId, vid_t _currentId, hid_t _hop):
        sourceId(_sourceId), currentId(_currentId), hop(_hop){}
};
 
template <typename WalkDataType>
class RandomWalk {
public:
    bid_t nblocks;
    vid_t *blocks;
    wid_t R;
    hid_t L;

    virtual void startWalks( WalkManager<WalkDataType> &walk_manager){
        logstream(LOG_ERROR) << "No definition of function : startWalks!" << std::endl;
    }  

    virtual void forwardWalk(WalkDataType walk, wid_t walkid, bid_t walkp, vid_t stv, vid_t env, eid_t *&beg_pos, vid_t *&csr, WalkManager<WalkDataType> &walk_manager ){
        logstream(LOG_FATAL) << "No definition of function : forwardWalk!" << std::endl;
    }

    virtual void updateInfo(vid_t s, vid_t dstId, tid_t threadid, hid_t hop){
        logstream(LOG_ERROR) << "No definition of function : updateInfo!" << std::endl;
    }

    virtual void compUtilization(eid_t total_edges, wid_t walksum, wid_t nwalks, double runtime){
        logstream(LOG_DEBUG) << "No definition of function : compUtilization!" << std::endl;
    }

    virtual void initializeRW( wid_t _R, hid_t _L) {
        R = _R;
        L = _L;
    }

    virtual void startWalks(WalkManager<WalkDataType> &walk_manager, bid_t _nblocks, vid_t* _blocks, std::string base_filename){
        nblocks = _nblocks;
        blocks = _blocks;
        startWalks(walk_manager);
    }

    virtual unsigned getblock( vid_t v ){
        for( unsigned p = 0; p < nblocks; p++ ){
            if( v < blocks[p+1] )
                return p;
        }
        return nblocks;
    }
    
    /**
     * check if it has finished all walks
     */
    virtual bool hasFinishedWalk(WalkManager<WalkDataType> &walk_manager){
        wid_t remaining_walknum = walk_manager.walksum;
        return ( remaining_walknum > 0 ); 
    }

    /**
     * compute the num of exec_blocks
     */
    virtual bid_t numExecBlocks(WalkManager<WalkDataType> &walk_manager, long long blocksize_kb){
        bid_t nexec_blocks = 1;
        wid_t remaining_walknum = walk_manager.walksum;
        long long comp_blocksize_kb = compBlockSize2(remaining_walknum);
        if(comp_blocksize_kb > blocksize_kb) 
            nexec_blocks = comp_blocksize_kb / blocksize_kb;
        return nexec_blocks; 
    }

    /**
     * determine the block size according to the number of walks
     * blocksize = 2^(log_2 R +1) MB
     */
    virtual unsigned long long compBlockSize2(wid_t nwalks){
        wid_t w = nwalks;
        int dom = 0;
        while(w > 1){
            dom++;
            w /= 2;
        }
        unsigned long long blocksize_kb = pow(2, dom);
        // unsigned long long blocksize_kb = pow(2, 1+dom);
        // unsigned long long blocksize_kb = pow(2, 3+dom);
        // logstream(LOG_DEBUG) << "nwalks = " << nwalks << ", dom = " << dom << ", determined blocksize_kb = " << blocksize_kb << std::endl;
        return blocksize_kb; 
    }

    /**
     * determine the block size according to the number of walks
     * blocksize = 2^(log_10 R + 2) MB
     */
    virtual unsigned long long compBlockSize(wid_t nwalks){
        wid_t w = nwalks;
        int dom = 0;
        while(w){
            dom++;
            w /= 10;
        }
        unsigned long long blocksize_kb = pow(2, 12 + dom);
        logstream(LOG_DEBUG) << "nwalks = " << nwalks << ", dom = " << dom << ", determined blocksize_kb = " << blocksize_kb << std::endl;
        return blocksize_kb; 
    }

    /**
     * determine the number of in-memory blocks
     */
    virtual bid_t compNmblocks(unsigned long long blocksize_kb){
        bid_t nmblocks = MEM_BUDGET / blocksize_kb;
        logstream(LOG_DEBUG) << "Computed nmblocks = " << nmblocks << std::endl;
        return nmblocks; 
    }
    
};

#endif