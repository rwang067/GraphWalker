
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

    //for convert_to_csr
    eid_t edgenum, max_edgenum; //total edges and max num of edges in an csr file
    vid_t vertnum, max_nvertices; //total vertices and max num of vertices in an beg_pos file

    bid_t bnum;
    std::vector<vid_t> blocks;
    vid_t bstv; //start vertex of current block
    vid_t curvert; // current vertex
    eid_t curpos; //current position in the current csr

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
     * Returns the number of blocks if a file has been already
     * 0 if not found.
     */
    static bid_t find_blockrange(std::string base_filename, uint16_t blocksize) {
        std::string blockrangefile = blockrangename(base_filename, blocksize);
        FILE *tryf = fopen(blockrangefile.c_str(), "r");
        if (tryf != NULL) { // Found!
            bid_t bnum = 0;
            while(!feof(tryf)){
                char flag = fgetc(tryf);
                if(flag == '\n')
                bnum++;
            }
            fclose(tryf);
            return bnum-1;
        }
        // Not found!
        logstream(LOG_WARNING) << "Could not find blocks with blocksize = " << blocksize << "MB." << std::endl;
        return 0;
    }

    void writeFileRange(std::string filename, uint16_t blocksize){
        /*write csr file range*/
        std::string filerangefile = blockrangename(filename, blocksize);
        std::ofstream frf(filerangefile.c_str());      
        for( bid_t p = 0; p < blocks.size(); p++ ){
            frf << blocks[p] << std::endl;
        }
        frf.close();

        /*write nvertices*/
        std::string nverticesfile = nverticesname(filename);
        std::ofstream nvf(nverticesfile.c_str());
        nvf << blocks[bnum] << std::endl;
        nvf.close();
    }

    void bwritezero( char * beg_pos, char * &beg_posptr, eid_t count ){
        while( count-- ){
            *((eid_t*)beg_posptr) = curpos;
            beg_posptr += sizeof(eid_t);
        }
    }

    void bwrite(char * beg_pos, char * &beg_posptr, char * csr, char * &csrptr, eid_t outd, std::vector<vid_t> outv, std::string filename ){
        curpos += outd;
        *((eid_t*)beg_posptr) = curpos;
        beg_posptr += sizeof(eid_t);
        for( eid_t i = 0; i < outd; i++ ){
            *((vid_t*)csrptr) = outv[i];
            csrptr += sizeof(vid_t);
        }
    }

    void flushBlock(std::string filename, char * csr, char * &csrptr, char * beg_pos, char * &beg_posptr){
        
        std::string fidfile = blockname(filename,bnum);
        std::string csrname = fidfile + ".csr";
        std::string beg_posname = fidfile + ".beg_pos";
        appendfile(csrname, csr, csrptr);
        appendfile(beg_posname, beg_pos, beg_posptr);

        logstream(LOG_INFO) << "FILE_" << bnum << " : [ " << bstv << " , " << curvert-1 << " ], totally stores " 
                            << curvert - bstv << " vertices, and " << curpos << " edges." << std::endl;

        csrptr = csr;
        beg_posptr = beg_pos + sizeof(eid_t);
        bstv = curvert;
        edgenum += curpos;
        curpos = 0;
    }

    bid_t convert_to_csr(std::string filename, uint16_t filesize_MB){

        FILE * inf = fopen(filename.c_str(), "r");
        if (inf == NULL) {
            logstream(LOG_FATAL) << "Could not load :" << filename << " error: " << strerror(errno) << std::endl;
        }
        assert(inf != NULL);

        rm_dir((filename+"_GraphWalker/").c_str());
        mkdir((filename+"_GraphWalker/").c_str(), 0777);
        mkdir((filename+"_GraphWalker/graphinfo/").c_str(), 0777);

        max_edgenum = (eid_t)filesize_MB * 1024 * 1024 / sizeof(vid_t); //max number of (vertices+edges) of a shard
        max_nvertices = (vid_t)max_edgenum / 8; //max number of (vertices+edges) of a shard
        logstream(LOG_INFO) << "Begin convert_to_csr, max_edgenum in an csr file = " << max_edgenum << ", max_nvertices = " << max_nvertices << std::endl;
        // logstream(LOG_INFO) << "max_nvertices in a beg_pos buffer = " << max_nvertices << ", max_edgenum in an csr buffer = " << max_edgenum << std::endl;
        
        char * csr = (char*) malloc(max_edgenum*sizeof(vid_t));
        char * csrptr = csr;
        char * beg_pos = (char*) malloc(max_nvertices*sizeof(eid_t));
        char * beg_posptr = beg_pos;
               
        logstream(LOG_INFO) << "Reading in edge list format!" << std::endl;

        bnum = 0;
        bstv = 0;
        blocks.push_back(bstv);
        curvert = 0;
        curpos = 0;
        vertnum = 0;
        edgenum = 0;

        *((eid_t*)beg_posptr) = 0;
        beg_posptr += sizeof(eid_t);

        vid_t max_vert = 0;
        eid_t outd = 0;        
        std::vector<vid_t> outv;

        char s[1024];
        while(fgets(s, 1024, inf) != NULL) {
            if (s[0] == '#') continue; // Comment
            if (s[0] == '%') continue; // Comment
            
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
            if( from == curvert ){
                outv.push_back(to);
                outd++;
            }else{  //a new vertex
                if( curpos + outd >= max_edgenum || curvert - bstv + 1 >= max_nvertices ){
                    if( outd > max_edgenum){
                        logstream(LOG_ERROR) << "Too small memory capacity with max_edgenum = " << max_edgenum << " to support larger ourdegree of vert " << curvert << ", with outdegree = " << outd << std::endl;
                        assert(false);
                    }
                    flushBlock(filename, csr, csrptr, beg_pos, beg_posptr);
                    blocks.push_back(bstv);
                    bnum++;
                }
                bwrite(beg_pos, beg_posptr, csr, csrptr, outd, outv, filename); //write a vertex to buffer
                if( from - curvert > 1 ){ //there are verts with zero out-links
                    vid_t remianzero = from-curvert-1;
                    vid_t remainsize = max_nvertices - (curvert - bstv);
                    while(remianzero > remainsize){
                        bwritezero( beg_pos, beg_posptr, remainsize ); 
                        flushBlock(filename, csr, csrptr, beg_pos, beg_posptr);
                        logstream(LOG_DEBUG) << remianzero << " , remainsize =  " << remainsize << std::endl;
                        remianzero -= remainsize;
                        remainsize = max_nvertices;
                    }
                    bwritezero( beg_pos, beg_posptr, remianzero ); 
                }
                curvert = from;
                outd = 1;
                outv.clear();
                outv.push_back(to);
            }
        }
        fclose(inf);
        bwrite(beg_pos, beg_posptr, csr, csrptr, outd, outv, filename);//write the last vertex to buffer

        if(max_vert > curvert){
            logstream(LOG_INFO) << "need bwritezero, as max_vert = " << max_vert << ", curvertex = " << curvert << std::endl;
            bwritezero( beg_pos, beg_posptr, max_vert - curvert );
        }       
        
        flushBlock(filename, csr, csrptr, beg_pos, beg_posptr);
        blocks.push_back(max_vert+1);
        bnum++;

        logstream(LOG_INFO) << "Partitioned csr file number : " << bnum << std::endl;
        writeFileRange(filename,filesize_MB);

        //output beg_pos information
        logstream(LOG_INFO) << "vertnum = " << max_vert+1 << ", " << "edgenum = " << edgenum << std::endl;

        if(csr!=NULL) free(csr);
        if(beg_pos!=NULL) free(beg_pos);


        return bnum;
    }

    /**
     * Covertnum graph from an edge list format. Input may contain
     * value for the edges. Self-edges are ignored.
     */

    bid_t convert_if_notexists(std::string basefilename, uint16_t blocksize) {
        assert(blocksize > 0);
        bid_t bnum = find_blockrange(basefilename, blocksize);
        /* Check if input file is already sharded */
        if(bnum > 0) {
            logstream(LOG_INFO) << "Found computed blocks for " << basefilename << ", blocksize = " << blocksize << "MB, num blocks=" << bnum << std::endl;
            //return nshards;
        }else{
            logstream(LOG_INFO) << "Did not find computed blocks for " << basefilename  << std::endl;
            logstream(LOG_INFO) << "Will try compute the blcok range now..." << std::endl;

            bnum = convert_to_csr(basefilename, blocksize); //compute_block(basefilename, blocksize);

            logstream(LOG_INFO) << "Successfully finished compute_block for " << basefilename << std::endl;
            logstream(LOG_INFO) << "computed " << bnum << " blocks." << std::endl;
        }
        return bnum;
    }

#endif

