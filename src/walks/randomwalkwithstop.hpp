#ifndef RANDOMWALKWITHSTOP
#define RANDOMWALKWITHSTOP

#include <string>
#include <fstream>
#include <time.h>

#include "walks/walk.hpp" 
#include "api/datatype.hpp"

/**
 * Type definitions. Remember to create suitable graph shards using the
 * Sharder-program.
 */
 
class RandomWalkwithStop : public RandomWalk {

    vid_t firstsource, numsources;
    wid_t walkspersource;
    hid_t maxwalklength;

public:
    void initializeRW(wid_t _nwalks, hid_t _maxwalklength) {
        nwalks = _nwalks;
        maxwalklength = _maxwalklength;
        std::cout << "initializeRW -- nwalks, maxwalklength : "  << nwalks << " " << maxwalklength << std::endl;
    }   

    void updateByWalk(WalkDataType walk, wid_t walkid, sid_t exec_interval, eid_t *&beg_pos, vid_t *&csr, WalkManager &walk_manager ){ //, VertexDataType* vertex_value){
        // logstream(LOG_INFO) << "updateByWalk in randomwalkwithstop." << std::endl;
        tid_t threadid = omp_get_thread_num();
        WalkDataType nowWalk = walk;
        vid_t sourId = walk_manager.getSourceId(nowWalk);
        vid_t dstId = walk_manager.getCurrentId(nowWalk) + intervals[exec_interval];
        hid_t hop = walk_manager.getHop(nowWalk);
        unsigned seed = (unsigned)(walkid+dstId+hop+(unsigned)time(NULL));
        while (dstId >= intervals[exec_interval] && dstId < intervals[exec_interval+1] && hop < maxwalklength ){
            // std::cout  << " -> " << dstId << " " << walk_manager.getSourceId(walk) << std::endl;
            updateInfo(sourId, dstId, threadid, hop);
            vid_t dstIdp = dstId - intervals[exec_interval];
            eid_t outd = beg_pos[dstIdp+1] - beg_pos[dstIdp];
            if (outd > 0 && (float)rand_r(&seed)/RAND_MAX > 0.15 ){
                eid_t pos = beg_pos[dstIdp] + ((eid_t)rand_r(&seed))%outd;
                // logstream(LOG_DEBUG) << "dstId = " << dstId << ",  pos = " << pos << ", csr[pos] = " << csr[pos] << std::endl;
                dstId = csr[pos];
            }else{
                return;
            }
            hop++;
            nowWalk++;
        }
        if( hop < maxwalklength ){
            sid_t p = getInterval( dstId );
            if(p>=nshards) return;
            walk_manager.moveWalk(nowWalk, p, threadid, dstId - intervals[p]);
            walk_manager.setMinStep( p, hop );
        }
    }

};

#endif