
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

    void importEdges(std::string filename, DynamicGraph *graph, metrics &m){
        FILE * inf = fopen(filename.c_str(), "r");
        if (inf == NULL) {
            logstream(LOG_FATAL) << "Could not load :" << filename << " error: " << strerror(errno) << std::endl;
        }
        assert(inf != NULL);

        eid_t InputSize = 16 * 1024 * 1024;
        std::pair<vid_t, vid_t> *edges = new std::pair<vid_t, vid_t>[InputSize];

        eid_t count = 0;
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

            // graph->addEdge(from, to);

            edges[count].first = from;
            edges[count].second = to;
            count++;
            if(count == InputSize){
                m.start_time("_addEdges_");
                for(eid_t e = 0; e < InputSize; e++){
                    graph->addEdge(edges[e].first, edges[e].second);
                }
                // logstream(LOG_INFO) << "Added " << count << " edges." << std::endl;
                count = 0;
                m.stop_time("_addEdges_");
            }

        }
        delete [] edges;
        graph->flush();
        graph->writeBlockRange();
        logstream(LOG_WARNING) << "All edges imported done, totally generated " << graph->nblocks << " blocks." << std::endl;
    }

};

#endif