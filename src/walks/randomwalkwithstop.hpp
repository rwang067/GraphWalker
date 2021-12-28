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

public:  

    void updateByWalk(WalkDataType walk, wid_t walkid, bid_t exec_block, vid_t stv, vid_t env, eid_t *&beg_pos, vid_t *&csr, WalkManager &walk_manager ){ //, VertexDataType* vertex_value){
        tid_t threadid = omp_get_thread_num();
        WalkDataType nowWalk = walk;
        vid_t sourId = walk_manager.getSourceId(nowWalk);
        vid_t dstId = walk_manager.getCurrentId(nowWalk) + blocks[exec_block];
        hid_t hop = walk_manager.getHop(nowWalk);
        unsigned seed = (unsigned)(walkid+dstId+hop+(unsigned)time(NULL));
        while (dstId >= stv && dstId < env && hop < L ){
            updateInfo(sourId, dstId, threadid, hop);
            vid_t dstIdp = dstId - blocks[exec_block];
            if(stv+1 == env) dstIdp = 0;
            eid_t outd = beg_pos[dstIdp+1] - beg_pos[dstIdp];
            if (outd > 0 && (float)rand_r(&seed)/RAND_MAX > 0.15 ){
                eid_t pos = beg_pos[dstIdp] - beg_pos[0] + ((eid_t)rand_r(&seed))%outd;
                dstId = csr[pos];
            }else{
                return;
            }
            hop++;
            // nowWalk++;
        }
        if(hop == 0){
            logstream(LOG_INFO) << walk.sourceId << " " << walk.currentId << " " << walk.hop << std::endl;
            logstream(LOG_FATAL) << dstId << " not in [" << stv << ", " << env << "), hop = " << hop << std::endl;
            return;
        }
        if( hop < L ){
            bid_t p = getblock( dstId );
            if(p>=nblocks) return;
            // walk_manager.setHop(nowWalk, hop);
            nowWalk = walk_manager.encode(sourId, dstId - blocks[p], hop);
            walk_manager.moveWalk(nowWalk, p, threadid, dstId - blocks[p]);
            walk_manager.setMinStep( p, hop );
            walk_manager.ismodified[p] = true;
        }
    }

};

#endif