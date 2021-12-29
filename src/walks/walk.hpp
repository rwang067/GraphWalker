#ifndef DEF_GRAPHWALKER_WALK
#define DEF_GRAPHWALKER_WALK

#include <iostream>
#include <cstdio>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <queue>

#include "metrics/metrics.hpp"
#include "api/filename.hpp"
#include "api/io.hpp"
#include "walks/walkbuffer.hpp"

template <typename WalkDataType>
class WalkManager
{
protected:
	std::string base_filename;
	bid_t nblocks;
	tid_t nthreads;
	metrics &m;
public:
	wid_t* walknum; //number of tptal walks of each block
	wid_t* dwalknum; //number of disk walks of each block
	hid_t* minstep;
	WalkBuffer<WalkDataType> **pwalks;

	bid_t curp; //current block id
	WalkDataType *curwalks; // all walks of current block
	wid_t walksum;

	bool* ismodified;

public:
	WalkManager(metrics &_m,bid_t _nblocks, tid_t _nthreads, std::string _base_filename):base_filename(_base_filename), nblocks(_nblocks), nthreads(_nthreads), m(_m){
		pwalks = new WalkBuffer<WalkDataType>*[nthreads];
		for(tid_t i = 0; i < nthreads; i++)
			pwalks[i] = new WalkBuffer<WalkDataType>[nblocks];

		walknum = (wid_t*)malloc(nblocks*sizeof(wid_t));
		dwalknum = (wid_t*)malloc(nblocks*sizeof(wid_t));
		minstep = (hid_t*)malloc(nblocks*sizeof(hid_t));
		memset(walknum, 0, nblocks*sizeof(wid_t));
		memset(dwalknum, 0, nblocks*sizeof(wid_t));
		memset(minstep, 0xffff, nblocks*sizeof(hid_t));
		walksum = 0;

		rm_dir((base_filename+"_GraphWalker/walks/").c_str());
		mkdir((base_filename+"_GraphWalker/walks/").c_str(), 0777);	

		ismodified = (bool*)malloc(nblocks*sizeof(bool));
		memset(ismodified, false, nblocks*sizeof(bool));
	}

	~WalkManager(){
		for(bid_t p = 0; p < nthreads; p++){
			if(pwalks[p] != NULL){
				delete [] pwalks[p];
			}
		}
		if(pwalks != NULL) delete [] pwalks;
		if(walknum != NULL) free(walknum);
		if(dwalknum != NULL) free(dwalknum);
		if(minstep != NULL) free(minstep);
	}

	void moveWalk( WalkDataType walk, bid_t p, tid_t t, vid_t toVertex ){
		if(pwalks[t][p].size_w == WALK_BUFFER_SIZE){
			writeWalks2Disk(t,p);
        }
        assert(pwalks[t][p].size_w < WALK_BUFFER_SIZE);
		pwalks[t][p].push_back( walk );
	}
	void writeWalks2Disk(tid_t t, bid_t p){
		std::string walksfile = walksname( base_filename, p );
		int f = open(walksfile.c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
		pwritea( f, &pwalks[t][p][0], pwalks[t][p].size_w*sizeof(WalkDataType) );
		dwalknum[p] += pwalks[t][p].size_w;
		pwalks[t][p].size_w = 0;
		close(f);
	}

	wid_t getCurrentWalks(bid_t p, bid_t nexec_blocks){
		m.start_time("3_getCurrentWalks");
		// compute nwalks
		wid_t nwalks = 0;
		for(bid_t b = 0; b < nexec_blocks; b++)
			nwalks += walknum[p+b];
		curwalks = (WalkDataType*)malloc(nwalks*sizeof(WalkDataType));
		wid_t off = 0;
		for(bid_t b = 0; b < nexec_blocks; b++){
			// load disk walks
			wid_t count = 0;
			if(dwalknum[p+b] > 0){
				readWalksfromDisk(p+b, off);
				count += dwalknum[p+b];
			}
			// copy memory walks
			for(tid_t t = 0; t < nthreads; t++){
				if(pwalks[t][p+b].size_w > 0){
                    wid_t size_w = pwalks[t][p+b].size_w;
					for(wid_t w = 0; w < size_w; w++){
						curwalks[off+count+w] = pwalks[t][p+b][w];
					}
					count += size_w;
				}
			}
			if (count != walknum[p+b]) {
				logstream(LOG_DEBUG) << "p+b = " << p+b << " : " << std::endl;
				for(tid_t t = 0; t < nthreads; t++){
					if(pwalks[t][p+b].size_w > 0)
						logstream(LOG_DEBUG) << "pwalks[" << (int)t << "][" << p+b << "].size_w = " << pwalks[t][p+b].size_w << std::endl;
				}
				logstream(LOG_DEBUG) << "dwalknum[p+b] = " << dwalknum[p+b] << std::endl;
				logstream(LOG_FATAL) << "read walks count = " << count << ", recorded walknum[p+b] = " << walknum[p+b] << std::endl;
			}
			off += count;
			dwalknum[p+b] = 0;
			for(tid_t t = 0; t < nthreads; t++){
				pwalks[t][p+b].size_w = 0;
			}
		}
		if (off != nwalks) {
			logstream(LOG_DEBUG) << "p = " << p << ", nexec_blocks = " << nexec_blocks << ", recorded nwalks = " << nwalks << std::endl;
			logstream(LOG_FATAL) << "read walks off = " << off << ", recorded nwalks = " << nwalks << std::endl;
		}
		m.stop_time("3_getCurrentWalks");
		return nwalks;
	}

	void readWalksfromDisk(bid_t p, wid_t off){
		std::string walksfile = walksname( base_filename, p );
		int f = open(walksfile.c_str(),O_RDWR, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
		if (f < 0) {
			logstream(LOG_FATAL) << "Could not load :" << walksfile << " error: " << strerror(errno) << std::endl;
		}
		assert(f > 0);
		preada(f, &curwalks[off], dwalknum[p]*sizeof(WalkDataType), 0);
    	ftruncate(f,0);
		close(f);
		unlink(walksfile.c_str()); 
	}

	void updateWalkNum(bid_t p, bid_t nexec_blocks){
		m.start_time("6_updateWalkNum");
		wid_t forwardWalks = 0;
		for(bid_t b = 0; b < nblocks; b++){
			if(ismodified[b] && (b < p || b >= p + nexec_blocks)){
				ismodified[b] = false;
				wid_t newwalknum = 0;
				newwalknum = dwalknum[b];
				for(tid_t t = 0; t < nthreads; t++){
					newwalknum += pwalks[t][b].size_w;
				}
				if(newwalknum < walknum[b]){
					logstream(LOG_FATAL) <<" b = " << b <<", newwalknum = " << newwalknum << ", walknum[b] = " << walknum[b] << std::endl;
				}
				forwardWalks += newwalknum - walknum[b];
				walknum[b] = newwalknum;
			}
		}
		walksum += forwardWalks;
		free(curwalks);
		curwalks = NULL;
		m.stop_time("6_updateWalkNum");
	}

	void clearWalkNum(bid_t p, bid_t nexec_blocks){
		for(bid_t b = 0; b < nexec_blocks; b++){
			walksum -= walknum[p+b];
			walknum[p+b] = 0;
			minstep[p+b] = 0xffff;
		}
	}

     void setMinStep(bid_t p, hid_t hop ){
		if(minstep[p] > hop)
		{
			#pragma omp critical
			{
				minstep[p] = hop;
			}
		}
     }

     bid_t blockWithMaxWalks(bid_t nexec_blocks){
		wid_t maxw = 0, maxp = 0;
		for(bid_t p = 0; p < nblocks; p+=nexec_blocks){
			wid_t count = 0;
			for(bid_t b = 0; b < nexec_blocks && p+b < nblocks; b++)
				count += walknum[p+b];
			if( maxw < count ){
				maxw = count;
				maxp = p;
			}
	   	}
		assert(maxw > 0);
		return maxp;
     }

	 bid_t blockWithMaxWalks(){
		wid_t maxw = 0, maxp = 0;
		for(bid_t p = 0; p < nblocks; p++) {
			if( maxw < walknum[p] ){
				maxw = walknum[p];
				maxp = p;
			}
	   	}
		return maxp;
     }

     bid_t blockWithMinStep(bid_t exec_block){
		hid_t mins = 0xffff, minp = 0;
		for(bid_t p = 0; p < nblocks; p++) {
			if( mins > minstep[p] ){
				mins = minstep[p];
				minp = p;
			}
	   	}
		if(walknum[minp] > 0)
			return minp;
		return blockWithMaxWalks(exec_block);
     }

     bid_t blockWithMaxWeight(){
		float maxwt = 0;
		bid_t maxp = 0;
		for(bid_t p = 0; p < nblocks; p++) {
			if(  maxwt < (float)walknum[p]/minstep[p] ){
				maxwt = (float)walknum[p]/minstep[p];
				maxp = p;
			}
	   	}
		return maxp;
     }

     bid_t blockWithRandom(){
		bid_t ranp = rand() % nblocks;
		return ranp;
     }

	bid_t chooseBlock(float prob, bid_t nexec_blocks){
		float cc = ((float)rand())/RAND_MAX;
		if( cc < prob ){
			return blockWithMinStep(nexec_blocks);
		}
		return blockWithMaxWalks(nexec_blocks);
	}

     void printWalksDistribution(bid_t exec_block){
		//print walk number decrease trend
		std::string walk_filename = base_filename + ".walks";
		std::ofstream ofs;
	    ofs.open(walk_filename.c_str(), std::ofstream::out | std::ofstream::app );
	   	wid_t sum = 0;
	  	for(bid_t p = 0; p < nblocks; p++) {
	      		sum += walknum[p];
	   	}
	  	ofs << exec_block << " \t " << walknum[exec_block] << " \t " << sum << std::endl;
	 	ofs.close();
     }    

};

#endif