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

template<class WalkDataType>
class RandomWalkwithRestartwithJoint : public RandomWalk<WalkDataType>{

public:
    wid_t R;
    hid_t L;

public:

    void initializeRW(wid_t _R, hid_t _L){
        R = _R;
        L = _L;
    }

    void updateByWalk(WalkDataType walk, wid_t walkid, bid_t exec_block, vid_t stv, vid_t env, eid_t *&beg_pos, vid_t *&csr, WalkManager<WalkDataType> &walk_manager ){
            //get current time in microsecond as seed to compute rand_r
            tid_t threadid = omp_get_thread_num();
            WalkDataType nowWalk = walk;
            vid_t sourId = nowWalk.sourceId;
            vid_t curId = nowWalk.currentId + this->blocks[exec_block];
            vid_t dstId = curId;
            hid_t hop = nowWalk.hop;
            // unsigned seed = (unsigned)std::chrono::high_resolution_clock::now().time_since_epoch().count();
            unsigned seed = (unsigned)(walkid+dstId+hop+(unsigned)time(NULL));
            while (dstId >= stv && dstId < env ){
                this->updateInfo(sourId, dstId, threadid, hop);
                vid_t dstIdp = dstId - this->blocks[exec_block];
                eid_t outd = beg_pos[dstIdp+1] - beg_pos[dstIdp];
                if (outd > 0 && (float)rand_r(&seed)/RAND_MAX > 0.15 ){
                    eid_t pos = beg_pos[dstIdp] - beg_pos[0] + ((eid_t)rand_r(&seed))%outd;
                    dstId = csr[pos];
                }else{
                    dstId = sourId;
                }
                hop++;
                if(hop%L == this->L-1) break;
            }
            if( hop%L != this->L-1 ){
                bid_t p = this->getblock( dstId );
                if(p >= this->nblocks) return;
                nowWalk = WalkDataType(sourId, dstId - this->blocks[p], hop);
                walk_manager.moveWalk(nowWalk, p, threadid, dstId - this->blocks[p]);
                walk_manager.setMinStep( p, hop );
                walk_manager.ismodified[p] = true;
            }
    }
};

#endif