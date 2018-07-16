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
    int nwalks; 
    int nsteps;
    int nvertices;
    int nshards;
    float tail;
    int tailwalknum;
    std::vector<std::pair<vid_t, vid_t> > intervals;

public:

    //for SimRank
    virtual void startWalksbyApp( WalkManager &walk_manager){
    }

    virtual void updateInfo(WalkManager &walk_manager, WalkDataType walk, vid_t dstId){
    } 

    virtual void updateInfo(vid_t dstId){
    }    

    virtual void updateInfo(vid_t dstId, unsigned d){
    }

    /**
     *  Walk update function.
     */
    //void updateByWalk(std::vector<graphchi_vertex<VertexDataType, EdgeDataType> > &vertices, vid_t vid, int sub_interval_st, int sub_interval_en, walkManager &walk_manager, graphchi_context &gcontext){
    virtual void updateByWalk(WalkDataType walk, int exec_interval, std::vector<Vertex > &vertices, WalkManager &walk_manager ){ //, VertexDataType* vertex_value){
    }

    virtual void initializeRW( int _nwalks, int _nsteps, float _tail) {
        nwalks = _nwalks;
        nsteps = _nsteps;
        tail = _tail;
        tailwalknum = (int)(nwalks*tail);
        std::cout << nwalks << " " << nsteps << " " << tail << std::endl;
    }

    virtual void startWalks( WalkManager &walk_manager , int _nvertices, std::vector<std::pair<vid_t, vid_t> > _intervals ){
        nvertices = _nvertices;
        intervals = _intervals;
        nshards = intervals.size();
        for( int i = 0; i < nshards; i++ ){
            walk_manager.walknum[i] = 0;
            walk_manager.minstep[i] = 0xffffffff;
        }
        startWalksbyApp(walk_manager);
    }

    virtual int getInterval( vid_t v ){
        for( int p = 0; p < nshards; p++ ){
            if( v <= intervals[p].second )
                return p;
        }
        return -1;
    }

    /**
     * check if it has finished all walks
     */
    virtual bool hasFinishedWalk(WalkManager &walk_manager){
        return ( walk_manager.walksum() > tailwalknum ); 
    }
    
    /**
     * Called before an execution interval is started.
     */
    virtual void before_exec_interval(vid_t window_st, vid_t window_en) {
    }
    
    /**
     * Called after an execution interval has finished.
     */
    virtual void after_exec_interval(vid_t window_st, vid_t window_en) {
    }
    
};

#endif