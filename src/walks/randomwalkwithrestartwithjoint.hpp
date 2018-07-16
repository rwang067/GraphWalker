#ifndef RANDOMWALKWITHRESTARTWITHJOINT
#define RANDOMWALKWITHRESTARTWITHJOINT

#include <string>
#include <fstream>
#include <time.h>

#include "walks/walk.hpp" 
#include "api/datatype.hpp"

/**
 * Type definitions. Remember to create suitable graph shards using the
 * Sharder-program.
 */
 
class RandomWalkwithRestartwithJoint : public RandomWalk {

private:
    vid_t source;
    int nsources;
    int walkspersource;
    int nsteps;
    std::vector<uint16_t> nowstep;
    float tail;
    int tailwalknum;
    std::vector<std::pair<vid_t, vid_t> > intervals;

public:
    void initializeRW(  vid_t _source, int _nsources, int _walkspersource, int _nsteps, float _tail) {
        tail = _tail;
        source = _source;
        nsources = _nsources;
        walkspersource = _walkspersource;
        // tailwalknum = (int)( nsources * walkspersource * tail );
        nsteps = _nsteps;
        nowstep.resize(nsources);
        for( int i = 0; i < nsources; i++ )
            nowstep[i] = 0;
        std::cout << source << " " << nsources << " " <<walkspersource << " " << nsteps << " " << tail << std::endl;
    }

    void startWalks( WalkManager &walk_manager , int _nvertices, std::vector<std::pair<vid_t, vid_t> > _intervals ){
        nvertices = _nvertices;
        intervals = _intervals;
        nshards = intervals.size();
        for( int i = 0; i < nshards; i++ ){
            walk_manager.walknum[i] = 0;
            walk_manager.minstep[i] = 0xfffffff;
        }
        startWalksbyApp(walk_manager);
    }

    void startWalksbyApp(WalkManager &walk_manager){
        float avgstep = ((float)1 / 0.15);
        int startnum = ((int)(nsteps / avgstep) + 1)*walkspersource;
        std::cout << "start num : " << startnum << std::endl;
        for( int i = 0; i < nsources; i++ ){
            for( int j = 0; j < startnum; j++ ){
                vid_t s = source + i;
                int p = getInterval(s);
                vid_t cur = s - intervals[p].first;
                WalkDataType walk = walk_manager.encode(i, cur, 0);
                walk_manager.walks[p].push_back(walk);
                walk_manager.minstep[p] = 0;
            }
        }
        walk_manager.freshIntervalWalks();
        tailwalknum = (int)( nsources * startnum * tail );
        std::cout << "tailwalknum : " << tailwalknum << std::endl;
    }
    
    /**
     *  Walk update function.
     */
    //void updateByWalk(std::vector<graphchi_vertex<VertexDataType, EdgeDataType> > &vertices, vid_t vid, int sub_interval_st, int sub_interval_en, walkManager &walk_manager, graphchi_context &gcontext){
    void updateByWalk(WalkDataType walk, int exec_interval, std::vector<Vertex > &vertices, WalkManager &walk_manager, VertexDataType* vertex_value){
            WalkDataType nowWalk = walk;
            vid_t dstId = walk_manager.getCurrentId(nowWalk) + intervals[exec_interval].first;
            int hop = walk_manager.getHop(nowWalk);
            int s = walk_manager.getSourceId(walk);
            while (dstId >= intervals[exec_interval].first && dstId <= intervals[exec_interval].second /*&& nowstep[s] < nsteps*/ ){
                // std::cout  << " -> " << dstId ;
                //std::cout  << " nowstep " << s << " " << nowstep[s] << std::endl;
                Vertex &nowVertex = vertices[dstId - intervals[exec_interval].first];
                if( hop > 0 )
                #pragma omp critical
                {
                    vertex_value[dstId - intervals[exec_interval].first]++;
                }
                #pragma omp critical
                {
                    nowstep[s]++;
                }
                //nowVertex.set_data(nowVertex.get_data()+1);
                float cc = ((float)rand())/RAND_MAX;
                if (nowVertex.outd > 0 && cc > 0.15 )
                    dstId = random_outneighbor(nowVertex);
                else
                    return ;//dstId = walk_manager.getSourceId(walk) + source;
                hop++;
            }
            // std::cout  << " to " << dstId << std::endl;
            //std::cout  << " hop " << hop << std::endl;
            if( nowstep[s] < nsteps ){
                int p = getInterval( dstId );
                #pragma omp critical
                {
                    walk_manager.moveWalktoHop(nowWalk, p, dstId - intervals[p].first , hop);
                }
                if( walk_manager.minstep[p] > hop )
                    #pragma omp critical
                    {
                        walk_manager.minstep[p] = hop;
                    }
            }
            // std::cout << " move walk " << nowWalk << "  " << dstId << "  " << hop << std::endl;
    }
    
};

#endif