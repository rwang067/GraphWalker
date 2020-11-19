
#ifndef DEF_DYNAMIC_GRAPH
#define DEF_DYNAMIC_GRAPH

#include <algorithm>

#include "engine/staticgraph.hpp"


class DynamicGraph : public StaticGraph {
public:     
    vid_t *ebuffer; //edge buffer
    vid_t *immutable_ebuffer; // immutable edge buffer for compaction
    eid_t ecap; //max number of edges in ebuffer and immutable_ebuffer
    eid_t esize; //number of edges in ebuffer 
    eid_t logcap;
        
public:
        
    DynamicGraph(std::string _base_filename, uint16_t _blocksize, bid_t _nblocks, metrics &_m, size_t buffersize = 0, size_t logsize = 0)
            : StaticGraph(_base_filename, _blocksize, _nblocks, _m){
        ecap = (buffersize * 1024 * 1024) / (sizeof(vid_t)*2);
        esize = 0;
        ebuffer = (vid_t*) malloc(ecap*sizeof(vid_t)*2);
        immutable_ebuffer = nullptr;
        logcap = (logsize * 1024 * 1024) / (sizeof(vid_t)*2);
        logstream(LOG_INFO) << "ecap = " << ecap << " edges(" << buffersize << "MB), logsize = " << logcap << "edges(" << logsize << "MB)." << std::endl;
    }
        
    virtual ~DynamicGraph() {
        if(immutable_ebuffer != nullptr)
            free(immutable_ebuffer);
        if(ebuffer != nullptr)
            free(ebuffer);
    }

    void addEdge(vid_t s = 0, vid_t d = 0, bool isDel = 0){
        if(esize >= ecap){
            immutable_ebuffer = ebuffer;
            flush();
        }
        ebuffer[2*esize] = s;
        ebuffer[2*esize+1] = d;
        esize++;
    }

    /***
    * Flush edge logs from memory buffer to disk log file
    * Incluing edge logs classification, each graph block equipment with a log file
    ***/
    void flush(){
        m.start_time("_flush_");

        bid_t cur_nblocks = nblocks;

        
        //1. malloc logs
        m.start_time("_flush_1_malloc_logs");
        vid_t** logs = (vid_t**)malloc(cur_nblocks*sizeof(vid_t*));
        eid_t* nlogs = (eid_t*)malloc(cur_nblocks*sizeof(eid_t));
        for(bid_t p = 0; p < cur_nblocks; p++){
            logs[p] = (vid_t*)malloc(ecap*sizeof(vid_t)*2);
            nlogs[p] = 0;
        }
        m.stop_time("_flush_1_malloc_logs");

        //2. edge log classification by block
        m.start_time("_flush_2_log_classification");
        for(eid_t e = 0; e < ecap; e++){
            bid_t p = getblock(immutable_ebuffer[2*e]);
            logs[p][2*nlogs[p]] = immutable_ebuffer[2*e];
            logs[p][2*nlogs[p]+1] = immutable_ebuffer[2*e+1];
            nlogs[p]++;
        }
        m.stop_time("_flush_2_log_classification");

        //3. write edge logs for each block
        m.start_time("_flush_3_write_logs");
        for(bid_t p = 0; p < cur_nblocks; p++){
            if(nlogs[p]>0){
                bid_t new_p = getblock(logs[p][0]);
                std::string logfile = blockname( base_filename, blocks[new_p] ) + ".log";

                appendfile(logfile, logs[p], 2 * nlogs[p] * sizeof(vid_t));

                size_t fsize = filesize(logfile);
                // logstream(LOG_WARNING) << logfile << ", filesize = " << fsize/(1024*1024) << "MB, nlogs[" << p << "] = " << nlogs[p] << std::endl;
                if(fsize/(sizeof(vid_t)*2) >= logcap){ 
                    compaction(new_p);
                    remove(logfile.c_str());
                }
            }
            free(logs[p]);
        }
        m.stop_time("_flush_3_write_logs");

        //4. free space
        m.start_time("_flush_4_free_logs");
        free(nlogs);
        free(logs);
        free(immutable_ebuffer);

        ebuffer = (vid_t*) malloc(ecap*sizeof(vid_t)*2);
        esize = 0;
        m.stop_time("_flush_4_free_logs");

        m.stop_time("_flush_");
    }

    void compaction(bid_t p){
        m.start_time("_compaction_");
        
        //1. Load the logs of block_p
        m.start_time("_compaction_1_loadLog");
        std::string logfile = blockname( base_filename, blocks[p] ) + ".log";
        vid_t* logs;
        eid_t nlogs = readfile(logfile, &logs) / (sizeof(vid_t)*2);
        m.stop_time("_compaction_1_loadLog");

        if(nlogs <= 0){
            logstream(LOG_ERROR) << p << " " << logfile << " "<< nlogs << std::endl;
        }
        assert(nlogs > 0);
        if(nlogs == 0) return; // no added edge logs of block p
        
        //2. malloc space for old CSR
        m.start_time("_compaction_2_mallocCSR");
        vid_t nverts = blocks[p+1] - blocks[p];
        eid_t nedges, *beg_pos = (eid_t*)malloc((nverts+1)*sizeof(eid_t));
        vid_t *csr = (vid_t*)malloc((size_t)blocksize*1024*1024);
        m.stop_time("_compaction_2_mallocCSR");

         //3. Load the CSR from disk
        m.start_time("_compaction_3_loadSubGraph");
        StaticGraph::loadSubGraph(p, beg_pos, csr, &nverts, &nedges);
        logstream(LOG_DEBUG) << "loaded " << nverts << " vertices and " << nedges << " edges of subgraph_" << p << ": [" << blocks[p] << ", " << blocks[p+1] << ")" << std::endl;
        m.stop_time("_compaction_3_loadSubGraph");

        //4. compute new degree for each block
        m.start_time("_compaction_4_computeDegree");
        eid_t* newdeg = (eid_t*)malloc(nverts*sizeof(eid_t));
        for(vid_t v = 0; v < nverts; v++){
            newdeg[v] = 0;
        }
        for(eid_t e = 0; e < nlogs; e++){
            vid_t v = logs[2*e]-blocks[p];
            if(!(v >= 0 && v < nverts)){
                logstream(LOG_WARNING) << v << " " << nverts << " " << getblock(logs[2*e]) << std::endl;
                for( bid_t b = 0; b < nblocks+1; b++ )
                    std::cout << blocks[b] << " ";
                std::cout << std::endl;
                // addEdge(logs[2*e], logs[2*e+1]);
                // continue;
            }
            assert(v >= 0 && v < nverts);
            newdeg[v]++;
        }

        m.stop_time("_compaction_4_computeDegree");

        //5. Allocate new space
        m.start_time("_compaction_5_mallocNewCSR");
        nedges += nlogs;
        vid_t * newcsr = (vid_t*) malloc(nedges*sizeof(vid_t));
        eid_t * newbeg_pos = (eid_t*) malloc((nverts+1)*sizeof(eid_t));
        m.stop_time("_compaction_5_mallocNewCSR");

        //6. Compute new begpos and copy CSR
        m.start_time("_compaction_6_compBeg+copyCSR");
        eid_t begoff = 0;
        newbeg_pos[0] = 0;
        for(vid_t v = 0; v < nverts; v++){
            if(newdeg[v] > 0){
                begoff += newdeg[v];
            }
            newbeg_pos[v+1] = beg_pos[v+1] + begoff;
            if(beg_pos[v+1]-beg_pos[v] > 0){
                memcpy(newcsr+newbeg_pos[v], csr+beg_pos[v], (beg_pos[v+1]-beg_pos[v])*sizeof(vid_t));
            }
        }
        // logstream(LOG_INFO) << "nverts = " << nverts << ", newbeg_pos[nverts] = " << newbeg_pos[nverts] << ", nedges = " << nedges << std::endl;
        assert(newbeg_pos[nverts] == nedges);
        m.stop_time("_compaction_6_compBeg+copyCSR");

        //7. write edge log to CSR
        m.start_time("_compaction_7_mergeLog2CSR");
        for(eid_t e = 0; e < nlogs; e++){
            vid_t v = logs[2*e] - blocks[p];
            assert(v >= 0 && v < nverts);
            // if(v == 0) logstream(LOG_INFO) << v << " --> " << logs[2*e+1] << ", deg-- = " << newdeg[v] << ", e = " << e << ", nlogs = " << nlogs << std::endl;
            assert(newdeg[v] > 0);
            eid_t pos = newbeg_pos[v+1] - newdeg[v];
            // if(v == 0 || v == 192) logstream(LOG_INFO) << v << " --> " << logs[2*e+1] << ", deg-- = " << newdeg[v] << ", newbeg_pos[v+1] = " << newbeg_pos[v+1] << ", pos = " << pos << std::endl;
            newcsr[pos] = logs[2*e+1];
            newdeg[v]--;
        }
        m.stop_time("_compaction_7_mergeLog2CSR");

        //8. Rewrite the CSR to disk
        m.start_time("_compaction_8_writeNewCsr");
        if( (size_t)(nedges*sizeof(vid_t)) >= (size_t)(blocksize * 1024 * 1024)){
            splitSubGraph(p, newcsr, nedges, newbeg_pos, nverts);
        }else{
            writeSubGraph(p, newcsr, nedges, newbeg_pos, nverts);
            // logstream(LOG_WARNING) << "After compaction, we have " << nverts << " vertices and " << nedges << " edges." << std::endl;
        }
        m.stop_time("_compaction_8_writeNewCsr");

        //9. Free the old CSR from memory
        m.start_time("_compaction_9_freeCSR");
        free(csr);
        free(beg_pos);
        csr = (vid_t*)newcsr;
        beg_pos = (eid_t*)newbeg_pos;
        if(logs != nullptr) free(logs);

        //10. Free new csr
        free(csr);
        free(beg_pos);
        m.stop_time("_compaction_9_freeCSR");
        
        m.stop_time("_compaction_");
    }

    void splitSubGraph(bid_t p, vid_t* csr, eid_t nedges, eid_t* beg_pos, vid_t nverts){
        std::vector<bid_t>::iterator it = blocks.begin() + p + 1;
        vid_t nverts1;
        eid_t nedges1;
        for(vid_t v = 0; v < nverts; v++){
            if(beg_pos[v] >= nedges/2){
                nverts1 = v+1;
                nedges1 = beg_pos[v];
                blocks.insert(it, blocks[p]+nverts1);
                nblocks++;
                break;
            }
        }
        logstream(LOG_INFO) << "split subgraph " << p << ": [" << blocks[p] << ", " << blocks[p+2] << "), into two subgraphs." << std::endl;
        writeSubGraph(p, csr, nedges1, beg_pos, nverts1);
        for(vid_t v = nverts1-1; v < nverts; v++){
            beg_pos[v] -= nedges1;
        }
        writeSubGraph(p+1, csr+nedges1, nedges-nedges1, beg_pos+nverts1-1, nverts-nverts1);
        logstream(LOG_INFO) << "After split, we have :" << std::endl;
        logstream(LOG_INFO) << " " << p << ": [" << blocks[p] << ", " << blocks[p+1] << "), #edges = " << nedges1 << std::endl;
        logstream(LOG_INFO) << " " << p+1 << ": [" << blocks[p+1] << ", " << blocks[p+2] << "), #edges = " << nedges-nedges1 << std::endl;
    }
    
    void loadSubGraph(bid_t p, eid_t * &beg_pos, vid_t * &csr, vid_t *nverts, eid_t *nedges){

        //3. Search logs of memory edge buffer
        /*
        vid_t* logs = (vid_t*)realloc(logs, (nlogs+esize)*sizeof(vid_t)*2);
        for(eid_t e = 0; e < esize; e++){
            if(getblock(ebuffer[2*e]) == p){
                logs[2*nlogs] = ebuffer[2*e];
                logs[2*nlogs+1] = ebuffer[2*e+1];
                nlogs++;
            }
        }
        */
    }

    std::vector<vid_t> getNeighbors(vid_t v){
        bid_t p = getblock(v);

        std::vector<vid_t> neighbors = StaticGraph::getNeighbors(v);
        // logstream(LOG_DEBUG) << "After loadBlock, # of neighbors = " << neighbors.size() << std::endl;

        // load and search logs of block_p
        std::string logfile = blockname( base_filename, blocks[p] ) + ".log";
        vid_t* logs;
        eid_t nlogs = readfile(logfile, &logs) / (sizeof(vid_t)*2);
        for(eid_t e = 0; e < nlogs; e++){
            if(logs[2*e] == v){
                neighbors.push_back(logs[2*e+1]);
            }
        }
        if(logs != nullptr) free(logs);
        // logstream(LOG_DEBUG) << "After loadLog, # of neighbors = " << neighbors.size() << std::endl;

        // search logs of memory edge buffer
        for(eid_t e = 0; e < esize; e++){
            if(ebuffer[2*e] == v){
                neighbors.push_back(ebuffer[2*e+1]);
            }
        }
        // logstream(LOG_DEBUG) << "After searchBuffer, # of neighbors = " << neighbors.size() << std::endl;

        return neighbors;
    }


};

#endif