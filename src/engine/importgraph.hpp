
#ifndef DEF_IMPORT_GRAPH
#define DEF_IMPORT_GRAPH


#include "engine/dynamicgraph.hpp"

class ImportGraph {
public:  

public:
        
    ImportGraph(){
    }
        
    virtual ~ImportGraph() {
    }

    void clearDir(std::string base_filename){
        rm_dir((base_filename+"_GraphWalker/").c_str());
        mkdir((base_filename+"_GraphWalker/").c_str(), 0777);
        mkdir((base_filename+"_GraphWalker/graphinfo/").c_str(), 0777);
    }

    void generateBlockRange(std::string base_filename, vid_t blocksize, bid_t nblocks, vid_t nverts){
        vid_t avg =  nverts / nblocks;
        std::string blockrangefile = blockrangename(base_filename, blocksize);
        std::ofstream frf(blockrangefile.c_str());      
        
        for( bid_t p = 0; p < nblocks; p++ ){
            frf << p * avg << std::endl;
        }
        frf << nverts << std::endl;
        frf.close();
    }

    void importEdges(std::string filename, DynamicGraph *graph){
        FILE * inf = fopen(filename.c_str(), "r");
        if (inf == NULL) {
            logstream(LOG_FATAL) << "Could not load :" << filename << " error: " << strerror(errno) << std::endl;
        }
        assert(inf != NULL);

        long long count = 0;
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

            graph->addEdge(from, to);

            if(count++ % (64*1024*1024) == 0)
                logstream(LOG_INFO) << "Processed " << count << " edges." << std::endl;
            // if(count == 1024*1024*1024)
            //     return;

        }
        graph->flush();
        logstream(LOG_WARNING) << "All edges imported done!" << std::endl;
    }

};

#endif