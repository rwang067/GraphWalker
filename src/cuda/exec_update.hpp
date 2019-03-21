#ifndef __EXEC_UPDATE__
#define __EXEC_UPDATE__

void exec_updates(metrics &_m, eid_t *beg_pos, vid_t *csr, sid_t exec_interval, vid_t* intervals, WalkDataType* walks, WalkDataType **&pwalks, wid_t *&pnwalks, vid_t nverts, eid_t nedges, wid_t nwalks, sid_t nshards);

#endif