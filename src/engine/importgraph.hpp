
#ifndef DEF_IMPORT_GRAPH
#define DEF_IMPORT_GRAPH


#include "engine/dynamicgraph.hpp"

class ImportGraph {
public:  
    metrics &m;

public:
        
    ImportGraph(metrics &_m):m(_m){
    }
        
    virtual ~ImportGraph() {
    }

    void clearDir(std::string base_filename){
        rm_dir((base_filename+"_GraphWalker/").c_str());
        mkdir((base_filename+"_GraphWalker/").c_str(), 0777);
        mkdir((base_filename+"_GraphWalker/graphinfo/").c_str(), 0777);
    }

    void generateBlockRange(std::string base_filename, vid_t N, vid_t nverts_per_blk){
        std::string blockrangefile = blockrangename(base_filename);
        std::ofstream frf(blockrangefile.c_str());      

        bid_t nblocks = N / nverts_per_blk + 1;
        for( bid_t p = 0; p < nblocks; p++ ){
            frf << p * nverts_per_blk << std::endl;
        }
        frf << N << std::endl;
        frf.close();
    }

    void addEdgeBuffer(std::pair<vid_t, vid_t> *edges, DynamicGraph *graph, eid_t nedges){
        m.start_time("_1_addEdges_");
        for(eid_t e = 0; e < nedges; e++){
            graph->addEdge(edges[e].first, edges[e].second);
        }
        // logstream(LOG_INFO) << "Added " << count << " edges." << std::endl;
        m.stop_time("_1_addEdges_");
    }

    void importEdgeList(std::string filename, DynamicGraph *graph){
        FILE * inf = fopen(filename.c_str(), "r");
        if (inf == NULL) {
            logstream(LOG_FATAL) << "Could not load :" << filename << " error: " << strerror(errno) << std::endl;
        }
        assert(inf != NULL);

        eid_t InputSize = 16 * 1024 * 1024;
        std::pair<vid_t, vid_t> *edges = new std::pair<vid_t, vid_t>[InputSize];

        eid_t count = 0;
        char s[1024];
        while( fgets(s, 1024, inf) != NULL && count < (258147869 * 0.8) ) {
        // while(fgets(s, 1024, inf) != NULL) {
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

            edges[count%InputSize].first = from;
            edges[count%InputSize].second = to;
            count++;
            if(count%InputSize == 0){
                addEdgeBuffer(edges, graph, InputSize);
            }
        }
        addEdgeBuffer(edges, graph, count%InputSize);
        delete [] edges;
        // graph->flush();
        // graph->writeLogfiles();
        graph->writeBlockRange();
        logstream(LOG_WARNING) << "All edges imported done, totally imported " << count << " edges, and generated " << graph->nblocks << " blocks." << std::endl;
    }

};

#endif