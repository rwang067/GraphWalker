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
	unsigned nshards;
	unsigned nthreads;
	metrics &m;
public:
	unsigned curp; //current interval
	unsigned* walknum;
	unsigned* minstep;
	VECTOR_W **pwalks;
public:
	WalkManager(metrics &_m,unsigned _nshards, unsigned _nthreads, std::string _base_filename):base_filename(_base_filename), nshards(_nshards), nthreads(_nthreads), m(_m){
		pwalks = new VECTOR_W*[nthreads];
		for(unsigned i = 0; i < nthreads; i++)
			pwalks[i] = new VECTOR_W[nshards];

		walknum = (unsigned*)malloc(nshards*sizeof(unsigned));
		minstep = (unsigned*)malloc(nshards*sizeof(unsigned));
		memset(walknum, 0, nshards*sizeof(unsigned));
		memset(minstep, 0x1f, nshards*sizeof(unsigned));
		mkdir((base_filename+"_GraphWalker/walks/").c_str(), 0777);	
	}
	~WalkManager(){
		for(unsigned p = 0; p < nthreads; p++)
			delete [] pwalks[p];
		delete [] pwalks;
		// free(pwalks);
		free(walknum);
		free(minstep);
	}

	WalkDataType encode( vid_t sourceId, vid_t currentId, unsigned hop ){
		assert( hop < 16384 );
		return (( (WalkDataType)sourceId & 0xffffff ) << 40 ) |(( (WalkDataType)currentId & 0x3ffffff ) << 14 ) | ( (WalkDataType)hop & 0x3fff ) ;
	}

	vid_t getSourceId( WalkDataType walk ){
		return (vid_t)( walk >> 40 ) & 0xffffff;
	}

	vid_t getCurrentId( WalkDataType walk ){
		return (vid_t)( walk >> 14 ) & 0x3ffffff;
	}

	unsigned getHop( WalkDataType walk ){
		return (unsigned)(walk & 0x3fff) ;
	}

	WalkDataType reencode( WalkDataType walk, vid_t toVertex ){
		unsigned hop = getHop(walk);
		unsigned source = getSourceId(walk);
		walk = encode(source,toVertex,hop);
		return walk;
	}

	void moveWalk( WalkDataType walk, unsigned p, unsigned t, vid_t toVertex ){
		walk = reencode( walk, toVertex );
		pwalks[t][p].push_back( walk );
	}

	void moveWalktoHop( WalkDataType walk, unsigned p, unsigned t, vid_t toVertex, unsigned hop ){
		walk = encode( getSourceId(walk), toVertex, hop );
		pwalks[t][p].push_back( walk );
	}

     unsigned walksum(){
		metrics_entry me = m.start_time();
		unsigned sum = 0;
		for(unsigned p=0; p < nshards; p++){
			sum += walknum[p];
		}
		m.stop_time(me, "_check-finish");
		// std::ofstream walkfile;
		// walkfile.open(base_filename + "_GraphWalker/remianing_walks.csv", std::ofstream::app);
		// walkfile << sum << "\n" ;
		// walkfile.close();
		return sum;
     }

     void setMinStep(unsigned p, unsigned hop ){
		if(minstep[p] > hop)
		{
			#pragma omp critical
			{
				minstep[p] = hop;
			}
		}
     }

     unsigned intervalWithMaxWalks(){
		metrics_entry me = m.start_time();
		unsigned maxw = 0, maxp = 0;
		for(unsigned p = 0; p < nshards; p++) {
			if( maxw < walknum[p] ){
				maxw = walknum[p];
				maxp = p;
			}
	   	}
		m.stop_time(me, "find-interval");
		return maxp;
     }

     unsigned intervalWithMinStep(){
		metrics_entry me = m.start_time();
		unsigned mins = 0xfffffff, minp = 0;
		for(unsigned p = 0; p < nshards; p++) {
			if( mins > minstep[p] ){
				mins = minstep[p];
				minp = p;
			}
	   	}
		m.stop_time(me, "find-interval");
		return minp;
     }

     unsigned intervalWithMaxWeight(){
		metrics_entry me = m.start_time();
		float maxwt = 0;
		unsigned maxp = 0;
		for(unsigned p = 0; p < nshards; p++) {
			if(  maxwt < (float)walknum[p]/minstep[p] ){
				maxwt = (float)walknum[p]/minstep[p];
				maxp = p;
			}
	   	}
		m.stop_time(me, "_find-interval-with-max-weight");
		return maxp;
     }

     unsigned intervalWithRandom(){
     		metrics_entry me = m.start_time();
     		unsigned ranp = rand() % nshards;
          	m.stop_time(me, "_find-interval-with-random");
          	return ranp;
     }

     void printWalksDistribution( unsigned exec_interval ){
		//print walk number decrease trend
		metrics_entry me = m.start_time();
		std::string walk_filename = base_filename + ".walks";
		std::ofstream ofs;
	    ofs.open(walk_filename.c_str(), std::ofstream::out | std::ofstream::app );
	   	unsigned sum = 0;
	  	for(unsigned p = 0; p < nshards; p++) {
	      		sum += walknum[p];
	   	}
	  	ofs << exec_interval << " \t " << walknum[exec_interval] << " \t " << sum << std::endl;
	 	ofs.close();
	 	m.stop_time(me, "_print-walks-distribution");
     }

     void readIntervalWalks( unsigned p ){
		m.start_time("readIntervalWalks");
        logstream(LOG_INFO) << "readIntervalWalks.." << std::endl;
		std::string walksfile = walksname( base_filename, p );
		unsigned f = open(walksfile.c_str(),O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
		if (f < 0) {
			logstream(LOG_FATAL) << "Could not load :" << walksfile << " error: " << strerror(errno) << std::endl;
		}
		assert(f > 0);
		size_t sz = readfull(f, &pwalks[0][p].walks);
		close(f);
		unsigned count = sz/sizeof(WalkDataType);
		pwalks[0][p].resize(count);
		pwalks[0][p].reserve(count);
		unsigned cap;
		if(nshards > 1) cap = count/(nshards-1)/nthreads + 1;
		else cap = count/nthreads + 1;
		for(unsigned i=0;i<nshards;i++){
			if(i!=p){
				for(unsigned t=0;t<nthreads;t++){
					pwalks[t][i].reserve(cap);
				}
			}
		}
        logstream(LOG_INFO) << "readIntervalWalks of p : " << p << " : " << count << std::endl;
		m.stop_time("readIntervalWalks");
     }

     void writeIntervalWalks_joint( unsigned p ){
		m.start_time("writeIntervalWalks");
		logstream(LOG_INFO) << "writeIntervalWalks.." << std::endl;
		//Clear walks of interval p in file
		std::string walksfile = walksname( base_filename, p );
		unsigned f = open(walksfile.c_str(), O_WRONLY | O_TRUNC, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
		if (f < 0) {
		    logstream(LOG_ERROR) << "Could not open " << walksfile << " error: " << strerror(errno) << std::endl;
		 }
		close(f);
		pwalks[0][p].clear();
		//Write walks of other intervals to file
		for( p = 0; p < nshards; p++){
			for(unsigned t = 1; t < nthreads; t++){
				if(!pwalks[t][p].isEmpty()){
					pwalks[0][p].joint(pwalks[t][p]);
					pwalks[t][p].clear();
				}
			}
			if(!pwalks[0][p].isEmpty()){
				std::string walksfile = walksname( base_filename, p );
				unsigned f = open(walksfile.c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
				if (f < 0)	logstream(LOG_ERROR) << "Could not open " << walksfile << " error: " << strerror(errno) << std::endl;
				writea( f, &pwalks[0][p][0], pwalks[0][p].size()*sizeof(WalkDataType));
				close(f);
				pwalks[0][p].clear();
			}
		}
		m.stop_time("writeIntervalWalks");
     }    

	 void writeIntervalWalks( unsigned p ){
		m.start_time("writeIntervalWalks");
		logstream(LOG_INFO) << "writeIntervalWalks of p : " << p << std::endl;
		//Clear walks of interval p in file
		std::string walksfile = walksname( base_filename, p );
		unsigned f = open(walksfile.c_str(), O_WRONLY | O_TRUNC, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
		if (f < 0) {
		    logstream(LOG_ERROR) << "Could not open " << walksfile << " error: " << strerror(errno) << std::endl;
		 }
		close(f);
		//Write walks of other intervals to file
		for( p = 0; p < nshards; p++){
			std::string walksfile = walksname( base_filename, p );
			unsigned f = open(walksfile.c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
			if (f < 0) {
				logstream(LOG_ERROR) << "Could not open " << walksfile << " error: " << strerror(errno) << std::endl;
			}
			for(unsigned t=0;t<nthreads;t++){
				if(!pwalks[t][p].isEmpty()){
					writea( f, &pwalks[t][p][0], pwalks[t][p].size()*sizeof(WalkDataType));
				}
				pwalks[t][p].clear();
			}
			close(f);
		}
		m.stop_time("writeIntervalWalks");
     }    

     void freshIntervalWalks( ){
		logstream(LOG_INFO) << "Write all started walks to files!" << std::endl;
		unsigned niothreads = get_option_int("niothreads");
		omp_set_num_threads(niothreads);
		#pragma omp parallel for schedule(dynamic, 1)
			for( unsigned p = 0; p < nshards; p++){
				std::string walksfile = walksname( base_filename, p );
				unsigned f = open(walksfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
				for(unsigned t=0;t<nthreads;t++){
					if(!pwalks[t][p].isEmpty()){
						pwritea( f, &pwalks[t][p][0], pwalks[t][p].size()*sizeof(WalkDataType) );
						pwalks[t][p].clear();
					}
				}
				close(f);
			}
	}    

      void freshIntervalWalks(unsigned p){
		logstream(LOG_INFO) << "Write started walks of interval " << p << " to files!" << std::endl;
		std::string walksfile = walksname( base_filename, p );
   		unsigned f = open(walksfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC| O_APPEND, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
		for(unsigned t=0;t<nthreads;t++){
			logstream(LOG_INFO) << pwalks[t][p].size() << std::endl;
			if(!pwalks[t][p].isEmpty()){
				pwritea( f, &pwalks[t][p][0], pwalks[t][p].size()*sizeof(WalkDataType) );
				pwalks[t][p].clear();
			}
		}
		close(f);
     }    

};

#endif