
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

    long long max_value(long long a, long long b){
        return (a > b ? a : b);
    }
    
    long long min_value(long long a, long long b){
        return (a < b ? a : b);
    }

    sid_t invlid;
    sid_t invlnum;
    vid_t stv, env;
    eid_t cur_pos;
    std::vector<vid_t> invls;

    int rm_dir(std::string dir_full_path){    
        DIR* dirp = opendir(dir_full_path.c_str());    
        if(!dirp){
            return -1;
        }
        struct dirent *dir;
        struct stat st;
        while((dir = readdir(dirp)) != NULL){
            if(strcmp(dir->d_name,".") == 0 || strcmp(dir->d_name,"..") == 0){
                continue;
            }    
            std::string sub_path = dir_full_path + '/' + dir->d_name;
            if(lstat(sub_path.c_str(),&st) == -1){
                // Log("rm_dir:lstat ",sub_path," error");
                logstream(LOG_WARNING) << "rm_dir:lstat " << sub_path << " error" << std::endl;
                continue;
            }    
            if(S_ISDIR(st.st_mode)){
                if(rm_dir(sub_path) == -1){ // 如果是目录文件，递归删除
                    closedir(dirp);
                    return -1;
                }
                rmdir(sub_path.c_str());
            }
            else if(S_ISREG(st.st_mode)) {
                unlink(sub_path.c_str());     // 如果是普通文件，则unlink
            }
            else{
                // Log("rm_dir:st_mode ",sub_path," error");
                logstream(LOG_WARNING) << "rm_dir:st_mode " << sub_path << " error" << std::endl;
                continue;
            }
        }
        if(rmdir(dir_full_path.c_str()) == -1){//delete dir itself.
            closedir(dirp);
            return -1;
        }
        closedir(dirp);
        return 0;
    }

    /**
     * Returns the number of shards if a file has been already
     * sharded or 0 if not found.
     */
    static sid_t find_shards(std::string base_filename, unsigned long long shardsize) {
        std::string intervalfname = filename_intervals(base_filename, shardsize);
        FILE *tryf = fopen(intervalfname.c_str(), "r");
        if (tryf != NULL) { // Found!
            sid_t nshards = 0;
            while(!feof(tryf)){
                char flag = fgetc(tryf);
                if(flag == '\n')
                nshards++;
            }
            fclose(tryf);
            return nshards-1;
        }
        // Not found!
        logstream(LOG_WARNING) << "Could not find shards with shardsize = " << shardsize << "KB." << std::endl;
        return 0;
    }

    void writeIntervals(std::string filename, unsigned long long shardsize){
        /*write intervals*/
        std::string intervalsFilename = filename_intervals(filename, shardsize);
        std::ofstream intervalsF(intervalsFilename.c_str());      
        for( sid_t p = 0; p < invls.size(); p++ ){
            intervalsF << invls[p] << std::endl;
        }
        intervalsF.close();

        /*write nvertices*/
        std::string nverticesFilename = filename_nvertices(filename);
        std::ofstream nverticesF(nverticesFilename.c_str());
        nverticesF << invls[invlnum] << std::endl;
        nverticesF.close();
    }

    void bwritezero( char * beg_pos, char * &beg_posptr, eid_t count ){
        env += count ;
        while( count-- ){
            *((eid_t*)beg_posptr) = cur_pos;
            beg_posptr += sizeof(eid_t);
        }
    }

    void bwrite(char * beg_pos, char * &beg_posptr, char * csr, char * &csrptr, eid_t count, std::vector<vid_t> outv, std::string filename ){
        cur_pos += count;
        *((eid_t*)beg_posptr) = cur_pos;
        beg_posptr += sizeof(eid_t);
        for( eid_t i = 0; i < count; i++ ){
            *((vid_t*)csrptr) = outv[i];
            csrptr += sizeof(vid_t);
        }
        env++;
    }

    void flushInvl(std::string filename, char * csr, char * &csrptr, char * beg_pos, char * &beg_posptr){
        std::string invlname = intervalname(filename,invlid);
        std::string csrname = invlname + ".csr";
        std::string beg_posname = invlname + ".beg_pos";
        writefile(csrname, csr, csrptr);
        writefile(beg_posname, beg_pos, beg_posptr);
        invls.push_back(env);
        logstream(LOG_INFO) << "interval_" << invlid << " : [ " << stv << " , " << env-1 << " ]" << std::endl;
        stv = env;
        invlid++;
        csrptr = csr;
        beg_posptr = beg_pos;
        cur_pos = 0;
        *((eid_t*)beg_posptr) = cur_pos;
        beg_posptr += sizeof(eid_t);
    }

    sid_t convert_by_shardsize(std::string filename, unsigned long long shardsize){

        eid_t max_nedges = shardsize * 1024 / sizeof(vid_t); //max number of (vertices+edges) of a shard
        vid_t max_nverts = max_nedges / 4;
        
        logstream(LOG_INFO) << "Begin convert_by_shardsize, max_nedges = " << max_nedges << ", max_nverts = " << max_nverts << std::endl;
        
        // return 0;
        FILE * inf = fopen(filename.c_str(), "r");
        if (inf == NULL) {
            logstream(LOG_FATAL) << "Could not load :" << filename << " error: " << strerror(errno) << std::endl;
        }
        assert(inf != NULL);

        rm_dir((filename+"_GraphWalker/").c_str());
        mkdir((filename+"_GraphWalker/").c_str(), 0777);
        mkdir((filename+"_GraphWalker/graphinfo/").c_str(), 0777);
        
        logstream(LOG_INFO) << "Reading in edge list format!" << std::endl;
        char * csr = (char*) malloc(max_nedges*sizeof(vid_t));
        char * csrptr = csr;
        char * beg_pos = (char*) malloc(max_nverts*sizeof(eid_t));
        char * beg_posptr = beg_pos;
        
        char s[1024];
        invlid = 0;
        vid_t curvertex = 0;
        eid_t count = 0;
        std::vector<vid_t> outv;
        stv = env = 0;
        invls.push_back(env);
        vid_t max_vert = 0;
        cur_pos = 0;
        *((eid_t*)beg_posptr) = cur_pos;
        beg_posptr += sizeof(eid_t);
        while(fgets(s, 1024, inf) != NULL) {
            if (s[0] == '#') continue; // Comment
            if (s[0] == '%') continue; // Comment
            
            // logstream(LOG_INFO) << " s= " << s << std::endl;
            char *t1, *t2;
            t1 = strtok(s, "\t, ");
            t2 = strtok(NULL, "\t, ");
            if (t1 == NULL || t2 == NULL ) {
                logstream(LOG_ERROR) << "Input file is not in right format. "
                << "Expecting <from> <to>. "
                << "Current line: " << s << "\n";
                assert(false);
            }
            vid_t from = atoi(t1);
            vid_t to = atoi(t2);
            if( from == to ) continue;
            max_vert = max_value(max_vert, from);
            max_vert = max_value(max_vert, to);
            if( from == curvertex ){
                outv.push_back(to);
                count++;
            }else{  //a new vertex
                if( (csrptr-csr)/sizeof(vid_t)+count >= max_nedges ){
                    // logstream(LOG_DEBUG) << "vert_id outd -- " << curvertex << ": " << count << std::endl;
                    flushInvl(filename, csr, csrptr, beg_pos, beg_posptr);
                    if( count > max_nedges){
                        logstream(LOG_ERROR) << "Too small shardsize with max_nedges = " << max_nedges << " to support larger ourdegree of vert " << curvertex << ", with outdegree = " << count << std::endl;
                        assert(false);
                    }
                }
                bwrite(beg_pos, beg_posptr, csr, csrptr, count, outv, filename); //write a vertex to buffer
                if( from - curvertex > 1 ){ //there are verts with zero out-links
                    vid_t remianzero = from-curvertex-1;
                    vid_t remainsize = max_nedges - ((csrptr-csr)/sizeof(vid_t));
                    // logstream(LOG_INFO) << "remianzero = " << remianzero << " , remainsize =  " << remainsize << " malloc_size = " << malloc_size << std::endl;
                    while(remianzero > remainsize){
                        bwritezero( beg_pos, beg_posptr, remainsize ); 
                        flushInvl(filename, csr, csrptr, beg_pos, beg_posptr);
                        logstream(LOG_DEBUG) << remianzero << " , remainsize =  " << remainsize << std::endl;
                        remianzero -= remainsize;
                        remainsize = max_nedges;
                    }
                    bwritezero( beg_pos, beg_posptr, remianzero ); 
                }
                curvertex = from;
                count = 1;
                outv.clear();
                outv.push_back(to);
            }
        }
        bwrite(beg_pos, beg_posptr, csr, csrptr, count, outv, filename);//write the last vertex to buffer
        fclose(inf);

        //output beg_pos information
        logstream(LOG_INFO) << "nverts = " << max_vert+1 << ", " << "nedges = " << (csrptr-csr)/sizeof(vid_t) << std::endl;
        logstream(LOG_INFO) << "env = " << env << ", beg_pos : "<< std::endl;
        for(vid_t i = env-10; i < env; i++)
            logstream(LOG_INFO) << "beg_pos[" << i << "] = " << *((eid_t*)(beg_pos+sizeof(eid_t)*i)) << ", " << *((eid_t*)(beg_pos+sizeof(eid_t)*(i+1))) << std::endl;

        if(max_vert > env-1){
            logstream(LOG_INFO) << "need bwritezero, as max_vert = " << max_vert << ", env = " << env << std::endl;
            bwritezero( beg_pos, beg_posptr, max_vert - (env-1) );
        }       
        
        flushInvl(filename, csr, csrptr, beg_pos, beg_posptr);

        invlnum = invlid;
        logstream(LOG_INFO) << "Partitioned interval number : " << invlnum << std::endl;

        if(csr!=NULL) free(csr);
        if(beg_pos!=NULL) free(beg_pos);

        writeIntervals(filename,shardsize);

        return invlnum;
    }

    /**
     * Converts graph from an edge list format. Input may contain
     * value for the edges. Self-edges are ignored.
     */

    sid_t convert_if_notexists(std::string basefilename, unsigned long long shardsize) {
        assert(shardsize > 0);
        sid_t nshards = find_shards(basefilename, shardsize);
        /* Check if input file is already sharded */
        if(nshards > 0) {
            logstream(LOG_INFO) << "Found preprocessed files for " << basefilename << ", shardsize = " << shardsize << "KB, num shards=" << nshards << std::endl;
            return nshards;
        }
        logstream(LOG_INFO) << "Did not find preprocessed shards for " << basefilename  << std::endl;
        // logstream(LOG_INFO) << "(Edge-value size: " << sizeof(EdgeDataType) << ")" << std::endl;
        logstream(LOG_INFO) << "Will try create them now..." << std::endl;

        nshards = convert_by_shardsize(basefilename, shardsize);

        logstream(LOG_INFO) << "Successfully finished sharding for " << basefilename << std::endl;
        logstream(LOG_INFO) << "Created " << nshards << " shards." << std::endl;
        return nshards;
    }

#endif

