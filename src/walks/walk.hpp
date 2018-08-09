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
	int nshards;
	int nthreads;
	metrics &m;
public:
	int curp; //current interval
	int* walknum;
	int* minstep;
	VECTOR_W **pwalks;
	// WalkDataType* curwalks;
	// std::vector<std::vector<WalkDataType> >  walks;
public:
	WalkManager(metrics &_m,int _nshards, int _nthreads, std::string _base_filename):base_filename(_base_filename), nshards(_nshards), nthreads(_nthreads), m(_m){
		// pwalks = (VECTOR_W**)malloc(nshards*sizeof(VECTOR_W*));
		// pwalks = new VECTOR_W*[nshards*nthreads];
		pwalks = new VECTOR_W*[nthreads];
		for(int i = 0; i < nthreads; i++)
			pwalks[i] = new VECTOR_W[nshards];
		// pwalks = new VECTOR_W*[nshards];
		// for(int i = 0; i < nshards; i++)
		// 	pwalks[i] = new VECTOR_W[nthreads];
		walknum = (int*)malloc(nshards*sizeof(int));
		minstep = (int*)malloc(nshards*sizeof(int));
		mkdir((base_filename+"_GraphWalker/walks/").c_str(), 0777);	
	}
	~WalkManager(){
		for(int p = 0; p < nthreads; p++)
			delete [] pwalks[p];
		delete [] pwalks;
		// free(pwalks);
		free(walknum);
		free(minstep);
	}

	WalkDataType encode( vid_t sourceId, vid_t currentId, int hop ){
		assert( hop < 16384 );
		return (( (WalkDataType)sourceId & 0xffffff ) << 40 ) |(( (WalkDataType)currentId & 0x3ffffff ) << 14 ) | ( (WalkDataType)hop & 0x3fff ) ;
	}

	vid_t getSourceId( WalkDataType walk ){
		return (vid_t)( walk >> 40 ) & 0xffffff;
	}

	vid_t getCurrentId( WalkDataType walk ){
		return (vid_t)( walk >> 14 ) & 0x3ffffff;
	}

	int getHop( WalkDataType walk ){
		return (int)(walk & 0x3fff) ;
	}

	WalkDataType reencode( WalkDataType walk, vid_t toVertex ){
		int hop = getHop(walk);
		int source = getSourceId(walk);
		walk = encode(source,toVertex,hop);
		return walk;
	}

	void moveWalk( WalkDataType walk, int p, int t, vid_t toVertex ){
		walk = reencode( walk, toVertex );
		// #pragma omp critical
        // {
			pwalks[t][p].push_back( walk );
		// }
	}

	void moveWalktoHop( WalkDataType walk, int p, int t, vid_t toVertex, int hop ){
		walk = encode( getSourceId(walk), toVertex, hop );
		// #pragma omp critical
        // {
			pwalks[t][p].push_back( walk );
		// }
	}

     int walksum(){
     		metrics_entry me = m.start_time();
     		int sum = 0;
          	for(int p=0; p < nshards; p++){
          		sum += walknum[p];
          	}
          	m.stop_time(me, "_check-finish");
          	return sum;
     }

     void setMinStep(int p, int hop ){
		if(minstep[p] > hop)
		{
			#pragma omp critical
			{
				minstep[p] = hop;
			}
		}
     }

     int intervalWithMaxWalks(){
		metrics_entry me = m.start_time();
		int maxw = 0, maxp = 0;
		for(int p = 0; p < nshards; p++) {
			logstream(LOG_INFO) << p << " : *w: " << walknum[p] << "   s: " << minstep[p] << std::endl ;
			if( maxw < walknum[p] ){
				maxw = walknum[p];
				maxp = p;
			}
	   	}
		m.stop_time(me, "find-interval");
		return maxp;
     }

     int intervalWithMinStep(){
		metrics_entry me = m.start_time();
		int mins = 0xfffffff, minp = 0;
		for(int p = 0; p < nshards; p++) {
			logstream(LOG_INFO) << p << " : w: " << walknum[p] << "   *s: " << minstep[p] << std::endl ;
			if( mins > minstep[p] ){
				mins = minstep[p];
				minp = p;
			}
	   	}
	   	logstream(LOG_INFO) << std::endl;
          	m.stop_time(me, "find-interval");
          	return minp;
     }

     int intervalWithMaxWeight(){
     		metrics_entry me = m.start_time();
     		float maxwt = 0;
     		int maxp = 0;
          	for(int p = 0; p < nshards; p++) {
	      		if(  maxwt < (float)walknum[p]/minstep[p] ){
          			maxwt = (float)walknum[p]/minstep[p];
          			maxp = p;
          		}
	   	}
          	m.stop_time(me, "_find-interval-with-max-weight");
          	return maxp;
     }

     int intervalWithRandom(){
     		metrics_entry me = m.start_time();
     		int ranp = rand() % nshards;
          	m.stop_time(me, "_find-interval-with-random");
          	return ranp;
     }

     void printWalksDistribution( int exec_interval ){
		//print walk number decrease trend
		metrics_entry me = m.start_time();
		std::string walk_filename = base_filename + ".walks";
		std::ofstream ofs;
	    ofs.open(walk_filename.c_str(), std::ofstream::out | std::ofstream::app );
	   	int sum = 0;
	  	for(int p = 0; p < nshards; p++) {
	      		sum += walknum[p];
	   	}
	  	ofs << exec_interval << " \t " << walknum[exec_interval] << " \t " << sum << std::endl;
	 	ofs.close();
	 	m.stop_time(me, "_print-walks-distribution");
     }

     size_t readIntervalWalks( int p ){
		m.start_time("readIntervalWalks");
		std::string walksfile = walksname( base_filename, p );
		int f = open(walksfile.c_str(),O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
		if (f < 0) {
			logstream(LOG_FATAL) << "Could not load :" << walksfile << " error: " << strerror(errno) << std::endl;
		}
		assert(f > 0);
		size_t sz = readfull(f, &pwalks[0][p].walks);
		close(f);
		int count = sz/sizeof(WalkDataType);
		pwalks[0][p].resize(count);
		pwalks[0][p].reserve(count);
		int cap = count/(nshards-1)/nthreads + 1;
		for(int i=0;i<nshards;i++){
			if(i!=p){
				for(int t=0;t<nthreads;t++){
					pwalks[t][i].reserve(cap);
				}
			}
		}
		m.stop_time("readIntervalWalks");
		return count;
     }

     void writeIntervalWalks( int p ){
		m.start_time("writeIntervalWalks");
		std::string walksfile = walksname( base_filename, p );
		int f = open(walksfile.c_str(), O_WRONLY | O_TRUNC, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
		if (f < 0) {
		    logstream(LOG_ERROR) << "Could not open " << walksfile << " error: " << strerror(errno) << std::endl;
		 }
		close(f);
		// free(curwalks);
		walknum[p] = 0;
		minstep[p] = 0xfffffff;
		pwalks[0][p].resize(0);
		for( p = 0; p < nshards; p++){
			std::string walksfile = walksname( base_filename, p );
			int f = open(walksfile.c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
			if (f < 0) {
			    logstream(LOG_ERROR) << "Could not open " << walksfile << " error: " << strerror(errno) << std::endl;
			}
			for(int t=0;t<nthreads;t++){
				if(!pwalks[t][p].isEmpty()){
					writea( f, &pwalks[t][p][0], pwalks[t][p].size()*sizeof(WalkDataType));
					walknum[p] += pwalks[t][p].size();
					pwalks[t][p].resize(0);
				}
			}
			close(f);

			// std::vector<WalkDataType> ().swap( walks[p] );
			// walks[p].clear();
			// walks[p].shrink_to_fit();
		}
		m.stop_time("writeIntervalWalks");
     }    

     void freshIntervalWalks( ){
		for( int p = 0; p < nshards; p++){
			std::string walksfile = walksname( base_filename, p );
			int f = open(walksfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
			for(int t=0;t<nthreads;t++){
				if(!pwalks[t][p].isEmpty()){
					pwritea( f, &pwalks[t][p][0], pwalks[t][p].size()*sizeof(WalkDataType) );
					walknum[p] += pwalks[t][p].size();
					pwalks[t][p].resize(0);
				}
			}
			close(f);
			// std::vector<WalkDataType> ().swap( walks[p] );
			// walks[p].clear();
			// walks[p].shrink_to_fit();
		}
	}    

      void freshIntervalWalks(int p){
		std::string walksfile = walksname( base_filename, p );
   		int f = open(walksfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
		for(int t=0;t<nthreads;t++){
			if(!pwalks[t][p].isEmpty()){
				pwritea( f, &pwalks[t][p][0], pwalks[t][p].size()*sizeof(WalkDataType) );
				walknum[p] += pwalks[t][p].size();
				pwalks[t][p].resize(0);
			}
		}
		close(f);
		// std::vector<WalkDataType> ().swap( walks[p] );
		// walks[p].clear();
		// walks[p].shrink_to_fit();
     }    

};

#endif