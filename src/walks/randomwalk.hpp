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
    wid_t nwalks;  //total walks
    hid_t nsteps;
    vid_t nvertices;
    sid_t nshards;
    vid_t *intervals;

public:

    //for SimRank
    virtual void startWalksbyApp( WalkManager &walk_manager){
    }

    virtual void updateInfo(WalkManager &walk_manager, WalkDataType walk, vid_t dstId){
    } 

    virtual void updateInfo(vid_t dstId){
    }  

    virtual void updateInfo(vid_t dstId, unsigned d, hid_t hop){
    }  

    virtual void updateInfo(vid_t sourId, vid_t dstId, unsigned d, hid_t hop){
    }

    /**
     *  Walk update function.
     */
    //void updateByWalk(std::vector<graphchi_vertex<VertexDataType, EdgeDataType> > &vertices, vid_t vid, unsigned sub_interval_st, unsigned sub_interval_en, walkManager &walk_manager, graphchi_context &gcontext){
    virtual void updateByWalk(WalkDataType walk, wid_t walkid, sid_t exec_interval, Vertex *&vertices, WalkManager &walk_manager ){ //, VertexDataType* vertex_value){
            logstream(LOG_ERROR) << "Not define updateByWalk!" << std::endl;
            assert(false);
    }

    virtual void updateByWalk(WalkDataType walk, wid_t walkid, sid_t exec_interval, eid_t *&beg_pos, vid_t *&csr, WalkManager &walk_manager ){ //, VertexDataType* vertex_value){
            logstream(LOG_ERROR) << "Not define updateByWalk!" << std::endl;
            assert(false);
    }

    virtual void initializeRW( wid_t _nwalks, hid_t _nsteps) {
        nwalks = _nwalks;
        nsteps = _nsteps;
    }

    virtual void startWalks( WalkManager &walk_manager , vid_t _nvertices, sid_t _nshards, vid_t* _intervals ){
        nvertices = _nvertices;
        nshards = _nshards;
        intervals = _intervals;
        startWalksbyApp(walk_manager);
    }

    virtual sid_t getInterval( vid_t v ){
        for( sid_t p = 1; p <= nshards; p++ ){
            if( v < intervals[p] )
                return p-1;
        }
        assert(false);
        return nshards;
    }
    
    /**
     * Called before an execution interval is started.
     */
    virtual void before_exec_interval(sid_t exec_interval, vid_t window_st, vid_t window_en, WalkManager &walk_manager) {
    }
    
    /**
     * Called after an execution interval has finished.
     */
    // virtual void after_exec_interval(unsigned exec_interval, vid_t window_st, vid_t window_en, WalkManager &walk_manager, Vertex *&vertices) {
    virtual void after_exec_interval(sid_t exec_interval, vid_t window_st, vid_t window_en, WalkManager &walk_manager) {
    }
    
};

#endif