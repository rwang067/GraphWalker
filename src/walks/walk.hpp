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
	sid_t nshards;
	metrics &m;
public:
	sid_t curp; //current interval
	wid_t* walknum;
	hid_t* minstep;
	WalkDataType *walks, **pwalks;
	wid_t nwalks, *pnwalks;

public:
	WalkManager(metrics &_m,sid_t _nshards, std::string _base_filename):base_filename(_base_filename), nshards(_nshards), m(_m){
		pwalks = (WalkDataType**)malloc(nshards*sizeof(WalkDataType*));
		pnwalks = (wid_t*)malloc(nshards*sizeof(wid_t));
		walknum = (wid_t*)malloc(nshards*sizeof(wid_t));
		minstep = (hid_t*)malloc(nshards*sizeof(hid_t));

		for(sid_t p =0; p < nshards; p++) {
			pwalks[p] = NULL;
			pnwalks[p] = 0;
			walknum[p] = 0;
			minstep[p] = 0;
		}

		walks = NULL;
		nwalks = 0;

		mkdir((base_filename+"_GraphWalker/walks/").c_str(), 0777);	
	}
	~WalkManager(){
		free(pwalks);
		free(pnwalks);
		free(walknum);
		free(minstep);
	}

	WalkDataType encode( vid_t sourceId, vid_t currentId, unsigned hop ){
		assert( hop < 16384 );
		return (( (WalkDataType)sourceId & 0xffffff ) << 40 ) |(( (WalkDataType)currentId & 0x3ffffff ) << 14 ) | ( (WalkDataType)hop & 0x3fff ) ;
	}

	void loadWalkPool(sid_t p){
		m.start_time("loadWalkPool");

		std::string walkname = walksname(base_filename, p);
		int walkf = open(walkname.c_str(),O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
		if (walkf < 0) {
			logstream(LOG_FATAL) << "Could not load :" << walkname << ", error: " << strerror(errno) << std::endl;
		}
		assert(walkf > 0);
		nwalks = readfull(walkf, &walks) / sizeof(WalkDataType);

		m.stop_time("loadWalkPool");
		// logstream(LOG_INFO) << "loadWalkPool end." << std::endl;
	}

	void writeWalkPools(){
		m.start_time("writeWalkPools");
		for(sid_t p =0; p < nshards; p++){
			if(pnwalks[p]>0){
				std::string walkname = walksname(base_filename, p);
				int walkf = open(walkname.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
				if (walkf < 0) {
					logstream(LOG_FATAL) << "Could not load :" << walkname << ", error: " << strerror(errno) << std::endl;
				}
				assert(walkf > 0);
				logstream(LOG_INFO) << "write walks of interval " << p << ", pnwalks[p] = " << pnwalks[p] << std::endl;
				writea( walkf, pwalks[p], pnwalks[p]*sizeof(WalkDataType) );
			}
		}
		logstream(LOG_INFO) << "updateStatistics" << std::endl;
		updateStatistics();
		logstream(LOG_INFO) << "writeWalkPools end." << std::endl;
		m.stop_time("writeWalkPools");
	}

	void updateStatistics(){
		for(sid_t p = 0; p < nshards; p++){
			if(pwalks[p] != NULL){
				walknum[p] += pnwalks[p];
				pnwalks[p] = 0;
				free(pwalks[p]);
				pwalks[p] = NULL;
			}
		}
		if(walks != NULL){
			nwalks = 0;
			free(walks);
			walks = NULL;
		}
	}

	wid_t walkSum(){
		wid_t res = 0;
		for(sid_t p = 0; p < nshards; p++){
			res += walknum[p];
		}
		logstream(LOG_INFO) << "walk remaining = " << res << std::endl;
		return res;
	}

	sid_t intervalWithMaxWalks(){
		sid_t res = 0;
		for(sid_t p = 1; p < nshards; p++){
			if(walknum[p] > walknum[res]) res = p;
		}
		return res;
	}

	void printWalksDistribution( sid_t exec_interval ){
		//print walk number decrease trend
		metrics_entry me = m.start_time();
		std::string walk_filename = base_filename + ".walks";
		std::ofstream ofs;
		ofs.open(walk_filename.c_str(), std::ofstream::out | std::ofstream::app );
		wid_t sum = 0;
		for(sid_t p = 0; p < nshards; p++) {
				sum += walknum[p];
		}
		ofs << exec_interval << " \t " << walknum[exec_interval] << " \t " << sum << std::endl;
		ofs.close();
		m.stop_time(me, "_print-walks-distribution");
	}

};

#endif