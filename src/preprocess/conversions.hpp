
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

    int invlid;
    int invlnum;
    vid_t stv, env;
    eid_t cur_pos;
    std::vector<std::pair<vid_t, vid_t> > invls;

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
    static int find_shards(std::string base_filename, long long shardsize) {
        std::string intervalfname = filename_intervals(base_filename, shardsize);
        FILE *tryf = fopen(intervalfname.c_str(), "r");
        if (tryf != NULL) { // Found!
            int nshards = 0;
            while(!feof(tryf)){
                char flag = fgetc(tryf);
                if(flag == '\n')
                nshards++;
            }
            fclose(tryf);
            return nshards;
        }
        // Not found!
        logstream(LOG_WARNING) << "Could not find shards with shardsize = " << shardsize << "KB." << std::endl;
        return 0;
    }

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

    void bwritezero( char * beg_pos, char * &beg_posptr, int count ){
        while( count-- ){
            *((eid_t*)beg_posptr) = cur_pos;
            beg_posptr += sizeof(eid_t);
        }
    }

    void bwrite(char * beg_pos, char * &beg_posptr, char * csr, char * &csrptr, int count, std::vector<vid_t> outv, std::string filename ){
        cur_pos += count;
        *((eid_t*)beg_posptr) = cur_pos;
        beg_posptr += sizeof(eid_t);
        for( int i = 0; i < count; i++ ){
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
        std::pair<vid_t, vid_t> invl(stv, env-1);
        invls.push_back(invl);
        logstream(LOG_INFO) << "interval_" << invlid << " : [ " << stv << " , " << env-1 << " ]" << std::endl;
        stv = env;
        invlid++;
        csrptr = csr;
        beg_posptr = beg_pos;
        cur_pos = 0;
        *((eid_t*)beg_posptr) = cur_pos;
        beg_posptr += sizeof(eid_t);
    }

    int convert_by_shardsize(std::string filename, long long shardsize){

        unsigned long long malloc_size = shardsize * 1024 / sizeof(int); //max number of (vertices+edges) of a shard
        
        logstream(LOG_INFO) << "Begin convert_by_shardsize, malloc_size = " << malloc_size << std::endl;
        
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
        char * csr = (char*) malloc(malloc_size*sizeof(vid_t));
        char * csrptr = csr;
        char * beg_pos = (char*) malloc((malloc_size/10)*sizeof(eid_t));
        char * beg_posptr = beg_pos;
        
        char s[1024];
        invlid = 0;
        vid_t curvertex = 0;
        eid_t count = 0;
        std::vector<vid_t> outv;
        stv = env = 0;
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
                if( (csrptr-csr)/sizeof(vid_t)+count >= malloc_size ){
                    // logstream(LOG_DEBUG) << "vert_id outd -- " << curvertex << ": " << count << std::endl;
                    flushInvl(filename, csr, csrptr, beg_pos, beg_posptr);
                    if( count > malloc_size){
                        logstream(LOG_ERROR) << "Too small shardsize with malloc_size = " << malloc_size << " to support larger ourdegree of vert " << curvertex << ", with outdegree = " << count << std::endl;
                        assert(false);
                    }
                }
                bwrite(beg_pos, beg_posptr, csr, csrptr, count, outv, filename); //write a vertex to buffer
                if( from - curvertex > 1 ){ //there are verts with zero out-links
                    vid_t remianzero = from-curvertex-1;
                    vid_t remainsize = malloc_size - ((csrptr-csr)/sizeof(int));
                    // logstream(LOG_INFO) << "remianzero = " << remianzero << " , remainsize =  " << remainsize << " malloc_size = " << malloc_size << std::endl;
                    while(remianzero > remainsize){
                        bwritezero( beg_pos, beg_posptr, remainsize ); 
                        env += remainsize ;
                        flushInvl(filename, csr, csrptr, beg_pos, beg_posptr);
                        logstream(LOG_DEBUG) << remianzero << " , remainsize =  " << remainsize << std::endl;
                        remianzero -= remainsize;
                        remainsize = malloc_size;
                    }
                    bwritezero( beg_pos, beg_posptr, remianzero ); 
                    env += remianzero ;
                }
                curvertex = from;
                count = 1;
                outv.clear();
                outv.push_back(to);     
            }
        }
        fclose(inf);
        bwrite(beg_pos, beg_posptr, csr, csrptr, count, outv, filename);
        if(max_vert > env-1) bwritezero( beg_pos, beg_posptr, max_vert - (env-1) ); 
        std::string invlname = intervalname(filename, invlid);
        std::string csrname = invlname + ".csr";
        std::string beg_posname = invlname + ".beg_pos";
        writefile(csrname, csr, csrptr);
        writefile(beg_posname, beg_pos, beg_posptr);
        free(csr);
        free(beg_pos);
        
        std::pair<vid_t, vid_t> invl(stv, max_vert);
        invls.push_back(invl);
        logstream(LOG_INFO) << "interval_" << invlid << " : [ " << stv << " , " << env-1 << " ]" << std::endl;
        invlnum = invlid+1;
        logstream(LOG_INFO) << "Partitioned interval number : " << invlnum << std::endl;

        /*write interval info*/
        std::string intervalsFilename = filename_intervals(filename, shardsize);
        std::ofstream intervalsF(intervalsFilename.c_str());      
        for( int p = 0; p < invlnum; p++ ){
            intervalsF << invls[p].second << std::endl;
        }
        intervalsF.close();

        /*write nvertices*/
        std::string nverticesFilename = filename_nvertices(filename);
        std::ofstream nverticesF(nverticesFilename.c_str());
        assert(max_vert == invls[invlnum-1].second);      
        nverticesF << max_vert+1 << std::endl;
        // nverticesF << invls[invlnum-1].second+1 << std::endl;
        intervalsF.close();
        return invlnum;
    }

    /**
     * Converts graph from an edge list format. Input may contain
     * value for the edges. Self-edges are ignored.
     */
    void convert_edgelist(std::string filename, int nshards, int nvertices, long long nedges) {
        unsigned long long malloc_size = (nvertices + nedges) / nshards + 1;
        logstream(LOG_INFO) << "malloc_size = " << malloc_size << std::endl;
        int invlnum = convert_by_shardsize(filename, malloc_size);
        assert(invlnum==nshards);
    }

    int convert_if_notexists(std::string basefilename, long long shardsize) {
        assert(shardsize > 0);
        int nshards = find_shards(basefilename, shardsize);
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
    
    int convert_if_notexists_nshards(std::string basefilename, std::string nshards_string, int nvertices, long long nedges, int nshards) {
        if(nshards == 0 ){
            double max_shardsize = 1024. * 1024. * size_t(get_option_int("membudget_mb", 1024)) / 4 ;
            nshards = (int) ( ((nedges+nvertices) * sizeof(VertexDataType) / max_shardsize) + 1.0);
            logstream(LOG_DEBUG) << "membudget_b nvertices nedges nshards : " << max_shardsize << " " << nvertices << " " << nedges << " " << nshards << std::endl;
        }
        /* Check if input file is already sharded */
        if ((nshards == find_shards(basefilename, nshards_string))) {
            logstream(LOG_INFO) << "Found preprocessed files for " << basefilename << ", num shards=" << nshards << std::endl;
            return nshards;
        }
        logstream(LOG_INFO) << "Did not find preprocessed shards for " << basefilename  << std::endl;
        // logstream(LOG_INFO) << "(Edge-value size: " << sizeof(EdgeDataType) << ")" << std::endl;
        logstream(LOG_INFO) << "Will try create them now..." << std::endl;

        convert_edgelist(basefilename, nshards, nvertices, nedges);

        logstream(LOG_INFO) << "Successfully finished sharding for " << basefilename << std::endl;
        logstream(LOG_INFO) << "Created " << nshards << " shards." << std::endl;
        return nshards;
    }

#endif

