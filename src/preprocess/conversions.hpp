
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
    vid_t stv, env;
    std::vector<std::pair<vid_t, vid_t> > invls;

    bool is_convert_by_walks;
    int num_verts, verts_per_invl;
    int num_edges, edges_per_invl;

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

    void bwritezero( char * buf, char * &bufptr, int count ){
        num_verts += count;
        while( count-- ){
            *((int*)bufptr) = 0;
            bufptr += sizeof(int);
        }
    }

    bool is_fill_up(){
        if(is_convert_by_walks && num_verts > verts_per_invl)
            return true;
        if(!is_convert_by_walks && num_edges >= edges_per_invl)
            return true;
        return false;
    }

    void bwrite( char * buf, char * &bufptr, int count, std::vector<vid_t> outv, std::string filename ){
        *((int*)bufptr) = count;
        bufptr += sizeof(int);
        for( int i = 0; i < count; i++ ){
            *((vid_t*)bufptr) = outv[i];
            bufptr += sizeof(vid_t);
        }
        num_verts++;
        num_edges += count;
        env++;

        //Judge if an interval buffer is fill up, if so write the buffer into file
        if( is_fill_up() ){
            std::string invlname = intervalname(filename,invlid);
            writefile(invlname, buf, bufptr);
            std::pair<vid_t, vid_t> invl(stv, env-1);
            invls.push_back(invl);
            logstream(LOG_INFO) << "interval_" << invlid << " : [ " << stv << " , " << env-1 << " ]" << std::endl;
            stv = env;
            invlid++;
            num_verts = num_edges = 0;
        }
    }

    /**
     * Converts graph from an edge list format. Input may contain
     * value for the edges. Self-edges are ignored.
     */
    void convert_edgelist(std::string filename, int nshards, int nvertices, int nedges) {
        num_verts = num_edges = 0;
        verts_per_invl = nvertices / nshards + 1;
        edges_per_invl = nedges / nshards + 1;
        logstream(LOG_INFO) << "verts/edges_per_invl : " << verts_per_invl << " " << edges_per_invl << std::endl;
        
        FILE * inf = fopen(filename.c_str(), "r");
        if (inf == NULL) {
            logstream(LOG_FATAL) << "Could not load :" << filename << " error: " << strerror(errno) << std::endl;
        }
        assert(inf != NULL);

        rm_dir((filename+"_GraphWalker/").c_str());
        mkdir((filename+"_GraphWalker/").c_str(), 0777);
        mkdir((filename+"_GraphWalker/graphinfo/").c_str(), 0777);
        
        logstream(LOG_INFO) << "Reading in edge list format!" << std::endl;

        char * buf = (char*) malloc((nvertices+nedges)*sizeof(int));
        char * bufptr = buf;
        
        char s[1024];
        invlid = 0;
        vid_t curvertex = 0;
        int count = 0;
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
                bwrite( buf, bufptr, count, outv, filename); //write a vertex to buffer
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
        bwrite( buf, bufptr, count, outv, filename);
        std::string invlname = intervalname(filename, invlid);
        writefile(invlname, buf, bufptr);
        std::pair<vid_t, vid_t> invl(stv, env-1);
        invls.push_back(invl);
        logstream(LOG_INFO) << "interval_" << invlid << " : [ " << stv << " , " << env-1 << " ]" << std::endl;
        invlnum = invlid+1;
        logstream(LOG_INFO) << "Partitioned interval number : " << invlnum << std::endl;

        /*write interval info*/
        std::string intervalsFilename = filename_intervals(filename, invlnum);
        std::ofstream intervalsF(intervalsFilename.c_str());      
        for( int p = 0; p < invlnum; p++ ){
            intervalsF << invls[p].second << std::endl;
        }
        intervalsF.close();
        free(buf);

        /*write nvertices*/
        std::string nverticesFilename = filename_nvertices(filename);
        std::ofstream nverticesF(nverticesFilename.c_str());      
        nverticesF << invls[invlnum-1].second+1 << std::endl;
        intervalsF.close();
        assert(invlnum==nshards);
    }
    
    int convert_if_notexists(std::string basefilename, std::string nshards_string, int nvertices, int nedges, int nwalks) {
        int nshards;
        unsigned membudget_b = 1024l * 1024l * size_t(get_option_int("membudget_mb", 1024));
        if( nwalks > nedges ) {
            nshards = nwalks*sizeof(WalkDataType)*2 / membudget_b + 1;
            is_convert_by_walks = true;
        }
        else{
            nshards = nedges*sizeof(VertexDataType)*2 / membudget_b + 1;
            is_convert_by_walks = false;
        }
        logstream(LOG_DEBUG) << "membudget_b nvertices nwalks nshards : " << membudget_b << " " << nvertices << " " << nwalks << " " << nshards << std::endl;

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

