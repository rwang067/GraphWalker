
#ifndef GRAPHWALKER_CONVERSIONS_DEF
#define GRAPHWALKER_CONVERSIONS_DEF

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#include <fstream>
#include <iostream>

#include "api/datatype.hpp"
#include "logger/logger.hpp"
#include "api/filename.hpp"
#include "api/io.hpp"

    int invlid;
    int invlnum;
    int invlsize;
    int cursize;
    vid_t stv, env;
    std::vector<std::pair<vid_t, vid_t> > invls;

    /**
     * Returns the number of shards if a file has been already
     * sharded or 0 if not found.
     */
    static int find_shards(std::string base_filename, std::string shard_string="auto") {
        int try_shard_num;
        int start_num = 0;
        int last_shard_num = 2400;
        if (shard_string == "auto") {
            start_num = 0;
        } else {
            start_num = atoi(shard_string.c_str());
        }
        
        if (start_num > 0) {
            last_shard_num = start_num;
        }
        
        for(try_shard_num=start_num; try_shard_num <= last_shard_num; try_shard_num++) {
            /* check the interval file */
            std::string intervalfname = filename_intervals(base_filename, try_shard_num);
            int tryf = open(intervalfname.c_str(), O_RDONLY);
            if (tryf >= 0) {
                // Found!
                close(tryf);
                int nshards_candidate = try_shard_num;
                bool success = true;
                // Validate all relevant files exists
                // logstream(LOG_INFO) << nshards_candidate << std::endl;

                for(int p=0; p < nshards_candidate; p++) {
                    std::string sname = intervalname(base_filename, p);
                    int tryf2 = open(sname.c_str(), O_RDONLY);
                    if (tryf2 < 0) {
                        logstream(LOG_DEBUG) << "Missing interval file: " << sname << std::endl;
                        success = false;
                        break;
                    }
                    close(tryf2);
                }
                if (!success) {
                    continue;
                }
                return nshards_candidate;
            }
        }
        if (last_shard_num == start_num) {
            logstream(LOG_WARNING) << "Could not find shards with nshards = " << start_num << std::endl;
            logstream(LOG_WARNING) << "Please define 'nshards 0' or 'nshards auto' to automatically detect." << std::endl;
        }
        return 0;
    }

    int computeInvlSize(){
        // membudget_mb = get_option_int("membudget_mb", 1024);
        invlsize = (1024l * 1024l * size_t(get_option_int("membudget_mb", 1024)) / 4l / sizeof(vid_t));
        // invlsize = 10*1024 * 1024 ;
        return invlsize;
    }

    void bwritezero( char * buf, char * &bufptr, int count ){
        while( count-- ){
            *((int*)bufptr) = 0;
            bufptr += sizeof(int);
        }
    }

    void bwrite( char * buf, char * &bufptr, int count, std::vector<vid_t> outv, std::string filename ){
        if( cursize + count + 1 > invlsize ){
            std::string invlname = intervalname(filename,invlid);
            writefile(invlname, buf, bufptr);
            std::pair<vid_t, vid_t> invl(stv, env-1);
            invls.push_back(invl);
            logstream(LOG_INFO) << invlid << " " << stv << " " << env-1 << std::endl;
            stv = env;
            invlid++;
            cursize = 0;
        }
        //if( count == 216 )
            //logstream(LOG_INFO) << "attention ! " << std::endl;
        *((int*)bufptr) = count;
        // if( env < 10 ) logstream(LOG_INFO) << *((int*)bufptr) << " : ";
        bufptr += sizeof(int);
        for( int i = 0; i < count; i++ ){
            *((vid_t*)bufptr) = outv[i];
             // if( env < 10 )  logstream(LOG_INFO) << *((int*)bufptr) << " ";
            bufptr += sizeof(vid_t);
        }
        // if( env < 10 )  logstream(LOG_INFO) << std::endl;
        cursize += count + 1;
        env++;
    }
    
    /**
     * Converts graph from an edge list format. Input may contain
     * value for the edges. Self-edges are ignored.
     */
    int convert_edgelist(std::string filename) {
        
        int invlsize = computeInvlSize();
        FILE * inf = fopen(filename.c_str(), "r");
        if (inf == NULL) {
            logstream(LOG_FATAL) << "Could not load :" << filename << " error: " << strerror(errno) << std::endl;
        }
        assert(inf != NULL);

        mkdir((filename+"_GraphWalker/").c_str(), 0777);
        mkdir((filename+"_GraphWalker/graphinfo/").c_str(), 0777);
        
        logstream(LOG_INFO) << "Reading in edge list format!" << std::endl;

        char * buf = (char*) malloc(2*invlsize*sizeof(int));
        char * bufptr = buf;
        
        char s[1024];
        invlid = 0;
        vid_t curvertex = 0;
        int count = 0;
        cursize = 0;
        std::vector<vid_t> outv;
        stv = env = 0;
        while(fgets(s, 1024, inf) != NULL) {
            if (s[0] == '#') continue; // Comment
            if (s[0] == '%') continue; // Comment
            
            char *t1, *t2;
            t1 = strtok(s, "\t, ");
            t2 = strtok(NULL, "\t, ");
            if (t1 == NULL || t2 == NULL ) {
                logstream(LOG_ERROR) << "Input file is not in right format. "
                << "Expecting \"<from>\t<to>\". "
                << "Current line: \"" << s << "\"\n";
                assert(false);
            }
            vid_t from = atoi(t1);
            vid_t to = atoi(t2);
            if( from == to ) continue;
            if( from == curvertex ){
                outv.push_back(to);
                count++;
            }else{
                bwrite( buf, bufptr, count, outv, filename);
                if( from - curvertex > 1 ){ 
                    bwritezero( buf, bufptr, from-curvertex-1 ); 
                    env += from - curvertex -1 ;
                }
                curvertex = from;
                count = 1;
                outv.clear();
                outv.push_back(to);     
            }
        }
        fclose(inf);
        logstream(LOG_INFO) << "count = " << count << std::endl;
        bwrite( buf, bufptr, count, outv, filename);
        std::string invlname = intervalname(filename, invlid);
        writefile(invlname, buf, bufptr);
        std::pair<vid_t, vid_t> invl(stv, env-1);
        invls.push_back(invl);
        logstream(LOG_INFO) << invlid << " " << stv << " " << env-1 << std::endl;
        invlnum = invlid+1;
        logstream(LOG_INFO) << "Partitioned interval number : " << invlnum << std::endl;

        /*write interval info*/
        std::string intervalsFilename = filename_intervals(filename, invlnum);
        std::ofstream intervalsF(intervalsFilename.c_str());      
        for( int p = 0; p < invlnum; p++ ){
            intervalsF << invls[p].second << std::endl;
        }
        intervalsF.close();

        /*write nvertices*/
        std::string nverticesFilename = filename_nvertices(filename);
        std::ofstream nverticesF(nverticesFilename.c_str());      
        nverticesF << invls[invlnum-1].second+1 << std::endl;
        intervalsF.close();
        
        return invlnum;
    }
 
    /**
     * Converts a graph input to shards. Preprocessing has several steps,
     * see sharder.hpp for more information.
     */
    int convert(std::string basefilename, std::string nshards_string) {
        int nshards = convert_edgelist(basefilename);    
        logstream(LOG_INFO) << "Successfully finished sharding for " << basefilename << std::endl;
        logstream(LOG_INFO) << "Created " << nshards << " shards." << std::endl;
        return nshards;
    }
    
    int convert_if_notexists(std::string basefilename, std::string nshards_string) {
        int nshards;
        
        /* Check if input file is already sharded */
        if ((nshards = find_shards(basefilename, nshards_string))) {
            logstream(LOG_INFO) << "Found preprocessed files for " << basefilename << ", num shards=" << nshards << std::endl;
            return nshards;
        }
        logstream(LOG_INFO) << "Did not find preprocessed shards for " << basefilename  << std::endl;
        // logstream(LOG_INFO) << "(Edge-value size: " << sizeof(EdgeDataType) << ")" << std::endl;
        logstream(LOG_INFO) << "Will try create them now..." << std::endl;
        nshards = convert(basefilename, nshards_string);
        return nshards;
    }

#endif

