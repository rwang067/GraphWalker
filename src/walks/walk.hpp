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
#include "walks/vector_w.hpp"

class WalkManager
{
protected:
	std::string base_filename;
	bid_t nshards;
	tid_t nthreads;
	metrics &m;
public:
	bid_t curp; //current block
	wid_t* walknum;
	hid_t* minstep;
	VECTOR_W **pwalks;
public:
	WalkManager(metrics &_m,bid_t _nshards, tid_t _nthreads, std::string _base_filename):base_filename(_base_filename), nshards(_nshards), nthreads(_nthreads), m(_m){
		pwalks = new VECTOR_W*[nthreads];
		for(tid_t i = 0; i < nthreads; i++)
			pwalks[i] = new VECTOR_W[nshards];

		walknum = (wid_t*)malloc(nshards*sizeof(wid_t));
		minstep = (hid_t*)malloc(nshards*sizeof(hid_t));
		memset(walknum, 0, nshards*sizeof(wid_t));
		memset(minstep, 0x1f, nshards*sizeof(hid_t));
		mkdir((base_filename+"_GraphWalker/walks/").c_str(), 0777);	
	}
	~WalkManager(){
		for(bid_t p = 0; p < nthreads; p++)
			delete [] pwalks[p];
		delete [] pwalks;
		// free(pwalks);
		free(walknum);
		free(minstep);
	}

	WalkDataType encode( vid_t sourceId, vid_t currentId, hid_t hop ){
		assert( hop < 16384 );
		return (( (WalkDataType)sourceId & 0xffffff ) << 40 ) |(( (WalkDataType)currentId & 0x3ffffff ) << 14 ) | ( (WalkDataType)hop & 0x3fff ) ;
	}

	vid_t getSourceId( WalkDataType walk ){
		return (vid_t)( walk >> 40 ) & 0xffffff;
	}

	vid_t getCurrentId( WalkDataType walk ){
		return (vid_t)( walk >> 14 ) & 0x3ffffff;
	}

	hid_t getHop( WalkDataType walk ){
		return (hid_t)(walk & 0x3fff) ;
	}

	WalkDataType reencode( WalkDataType walk, vid_t toVertex ){
		hid_t hop = getHop(walk);
		vid_t source = getSourceId(walk);
		walk = encode(source,toVertex,hop);
		return walk;
	}

	void moveWalk( WalkDataType walk, bid_t p, tid_t t, vid_t toVertex ){
		walk = reencode( walk, toVertex );
		pwalks[t][p].push_back( walk );
	}

	void moveWalktoHop( WalkDataType walk, bid_t p, tid_t t, vid_t toVertex, hid_t hop ){
		walk = encode( getSourceId(walk), toVertex, hop );
		pwalks[t][p].push_back( walk );
	}

     wid_t walksum(){
		metrics_entry me = m.start_time();
		wid_t sum = 0;
		for(bid_t p=0; p < nshards; p++){
			sum += walknum[p];
		}
		m.stop_time(me, "_check-finish");
		// std::ofstream walkfile;
		// walkfile.open("remianing_walks.csv", std::ofstream::app);
		// walkfile << sum << "\n" ;
		// walkfile.close();
		return sum;
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

     bid_t blockWithMaxWalks(){
		metrics_entry me = m.start_time();
		wid_t maxw = 0, maxp = 0;
		for(bid_t p = 0; p < nshards; p++) {
			if( maxw < walknum[p] ){
				maxw = walknum[p];
				maxp = p;
			}
	   	}
		m.stop_time(me, "find-block");
		return maxp;
     }

     bid_t blockWithMinStep(){
		metrics_entry me = m.start_time();
		hid_t mins = 0xffff, minp = 0;
		for(bid_t p = 0; p < nshards; p++) {
			if( mins > minstep[p] ){
				mins = minstep[p];
				minp = p;
			}
	   	}
		m.stop_time(me, "find-block");
		return minp;
     }

     bid_t blockWithMaxWeight(){
		metrics_entry me = m.start_time();
		float maxwt = 0;
		bid_t maxp = 0;
		for(bid_t p = 0; p < nshards; p++) {
			if(  maxwt < (float)walknum[p]/minstep[p] ){
				maxwt = (float)walknum[p]/minstep[p];
				maxp = p;
			}
	   	}
		m.stop_time(me, "_find-block-with-max-weight");
		return maxp;
     }

     bid_t blockWithRandom(){
     		metrics_entry me = m.start_time();
     		bid_t ranp = rand() % nshards;
          	m.stop_time(me, "_find-block-with-random");
          	return ranp;
     }

     void printWalksDistribution(bid_t exec_block ){
		//print walk number decrease trend
		metrics_entry me = m.start_time();
		std::string walk_filename = base_filename + ".walks";
		std::ofstream ofs;
	    ofs.open(walk_filename.c_str(), std::ofstream::out | std::ofstream::app );
	   	wid_t sum = 0;
	  	for(bid_t p = 0; p < nshards; p++) {
	      		sum += walknum[p];
	   	}
	  	ofs << exec_block << " \t " << walknum[exec_block] << " \t " << sum << std::endl;
	 	ofs.close();
	 	m.stop_time(me, "_print-walks-distribution");
     }

     wid_t readblockWalks( unsigned p ){
		m.start_time("readblockWalks");
        // logstream(LOG_INFO) << "readblockWalks.." << std::endl;
		std::string walksfile = walksname( base_filename, p );
		int f = open(walksfile.c_str(),O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
		if (f < 0) {
			logstream(LOG_FATAL) << "Could not load :" << walksfile << " error: " << strerror(errno) << std::endl;
		}
		assert(f > 0);
		size_t sz = readfull(f, &pwalks[0][p].walks);
		close(f);
		wid_t count = sz/sizeof(WalkDataType);
		pwalks[0][p].resize(count);
		pwalks[0][p].reserve(count);
		wid_t cap;
		if(nshards > 1) cap = count/(nshards-1)/nthreads + 1;
		else cap = count/nthreads + 1;
		for(bid_t i=0;i<nshards;i++){
			if(i!=p){
				for(tid_t t=0;t<nthreads;t++){
					pwalks[t][i].reserve(cap);
				}
			}
		}
        // logstream(LOG_INFO) << "readblockWalks of p : " << p << " : " << count << std::endl;
		m.stop_time("readblockWalks");
		return count;
     }

     void writeblockWalks_joint(bid_t p ){
		m.start_time("writeblockWalks");
		logstream(LOG_INFO) << "writeblockWalks.." << std::endl;
		//Clear walks of block p in file
		std::string walksfile = walksname( base_filename, p );
		int f = open(walksfile.c_str(), O_WRONLY | O_TRUNC, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
		if (f < 0) {
		    logstream(LOG_ERROR) << "Could not open " << walksfile << " error: " << strerror(errno) << std::endl;
		 }
		close(f);
		pwalks[0][p].clear();
		//Write walks of other blocks to file
		for( p = 0; p < nshards; p++){
			for(tid_t t = 1; t < nthreads; t++){
				if(!pwalks[t][p].isEmpty()){
					pwalks[0][p].joint(pwalks[t][p]);
					pwalks[t][p].clear();
				}
			}
			if(!pwalks[0][p].isEmpty()){
				std::string walksfile = walksname( base_filename, p );
				int f = open(walksfile.c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
				if (f < 0)	logstream(LOG_ERROR) << "Could not open " << walksfile << " error: " << strerror(errno) << std::endl;
				pwritea( f, &pwalks[0][p][0], pwalks[0][p].size()*sizeof(WalkDataType));
				close(f);
				pwalks[0][p].clear();
			}
		}
		m.stop_time("writeblockWalks");
     }    

	 void writeblockWalks( unsigned p ){
		m.start_time("writeblockWalks");
		// logstream(LOG_INFO) << "writeblockWalks of p : " << p << std::endl;
		//Clear walks of block p in file
		std::string walksfile = walksname( base_filename, p );
		int f = open(walksfile.c_str(), O_WRONLY | O_TRUNC, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
		if (f < 0) {
		    logstream(LOG_ERROR) << "Could not open " << walksfile << " error: " << strerror(errno) << std::endl;
		 }
		close(f);
		//Write walks of other blocks to file
		for( p = 0; p < nshards; p++){
			std::string walksfile = walksname( base_filename, p );
			int f = open(walksfile.c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
			if (f < 0) {
				logstream(LOG_ERROR) << "Could not open " << walksfile << " error: " << strerror(errno) << std::endl;
			}
			for(tid_t t=0;t<nthreads;t++){
				if(!pwalks[t][p].isEmpty()){
					pwritea( f, &pwalks[t][p][0], pwalks[t][p].size()*sizeof(WalkDataType));
				}
				pwalks[t][p].clear();
			}
			close(f);
		}
		m.stop_time("writeblockWalks");
     }    

     void freshblockWalks( ){
		logstream(LOG_INFO) << "Write all started walks to files!" << std::endl;
		#pragma omp parallel for schedule(dynamic, 1)
			for( bid_t p = 0; p < nshards; p++){
				std::string walksfile = walksname( base_filename, p );
				int f = open(walksfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
				for(tid_t t=0;t<nthreads;t++){
					if(!pwalks[t][p].isEmpty()){
						pwritea( f, &pwalks[t][p][0], pwalks[t][p].size()*sizeof(WalkDataType) );
						pwalks[t][p].clear();
					}
				}
				close(f);
			}
	}    

      void freshblockWalks(bid_t p){
		// logstream(LOG_INFO) << "Write started walks of block " << p << " to files!" << std::endl;
		std::string walksfile = walksname( base_filename, p );
   		int f = open(walksfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC| O_APPEND, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
		for(tid_t t=0;t<nthreads;t++){
			// logstream(LOG_INFO) << pwalks[t][p].size() << std::endl;
			if(!pwalks[t][p].isEmpty()){
				pwritea( f, &pwalks[t][p][0], pwalks[t][p].size()*sizeof(WalkDataType) );
				pwalks[t][p].clear();
			}
		}
		close(f);
     }    

};

#endif