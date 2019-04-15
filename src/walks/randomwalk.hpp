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
    sid_t nshards;
    vid_t *intervals;
    wid_t nwalks;

public:

    //for SimRank
    virtual void startWalksbyApp( WalkManager &walk_manager){
        logstream(LOG_ERROR) << "No definition of function : startWalksbyApp!" << std::endl;
    }

    virtual void updateInfo(WalkManager &walk_manager, WalkDataType walk, vid_t dstId){
    } 

    virtual void updateInfo(vid_t dstId){
    }  

    virtual void updateInfo(vid_t dstId, vid_t d, hid_t hop){
    }  

    virtual void updateInfo(vid_t sourId, vid_t dstId, vid_t d, hid_t hop){
    }

    /**
     *  Walk update function.
     */
    virtual void updateByWalk(WalkDataType walk, wid_t walkid, sid_t exec_interval, eid_t *&beg_pos, vid_t *&csr, WalkManager &walk_manager ){ //, VertexDataType* vertex_value){
        logstream(LOG_ERROR) << "No definition of function : updateByWalk!" << std::endl;
    }

    virtual void initializeRW( wid_t _nwalks, hid_t _nsteps) {
        logstream(LOG_DEBUG) << "No definition of function : initializeRW!" << std::endl;
    }
    
    /**
     * Called before an execution interval is started.
     */
    virtual void before_exec_interval(sid_t exec_interval, vid_t window_st, vid_t window_en, WalkManager &walk_manager) {
        logstream(LOG_DEBUG) << "No definition of function : before_exec_interval!" << std::endl;
    }
    
    /**
     * Called after an execution interval has finished.
     */
    // virtual void after_exec_interval(unsigned exec_interval, vid_t window_st, vid_t window_en, WalkManager &walk_manager, Vertex *&vertices) {
    virtual void after_exec_interval(sid_t exec_interval, vid_t window_st, vid_t window_en, WalkManager &walk_manager) {
        logstream(LOG_DEBUG) << "No definition of function : after_exec_interval!" << std::endl;
    }

    virtual void startWalks( WalkManager &walk_manager , sid_t _nshards, vid_t* _intervals ){
        nshards = _nshards;
        intervals = _intervals;
        for( sid_t i = 0; i < nshards; i++ ){
            walk_manager.walknum[i] = 0;
            walk_manager.minstep[i] = 0xffff;
        }
        startWalksbyApp(walk_manager);
    }

    virtual unsigned getInterval( vid_t v ){
        for( unsigned p = 0; p < nshards; p++ ){
            if( v < intervals[p+1] )
                return p;
        }
        logstream(LOG_DEBUG) << "v = " << v << ", intervals[nshards] = " << intervals[nshards] << std::endl;
        // assert(false);
        return nshards;
    }
    
    /**
     * check if it has finished all walks
     */
    virtual bool hasFinishedWalk(WalkManager &walk_manager){
        unsigned remaining_walknum = walk_manager.walksum();
        logstream(LOG_DEBUG) << "Walks remaining = " << remaining_walknum << std::endl;
        return ( remaining_walknum > 0 ); 
    }
    
};

#endif