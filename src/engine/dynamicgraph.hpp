
#ifndef DEF_DYNAMIC_GRAPH
#define DEF_DYNAMIC_GRAPH

#include <algorithm>

#include "engine/staticgraph.hpp"

class DynamicGraph : public StaticGraph {
public:     
    vid_t *ebuffer; //edge buffer
    vid_t *immutable_ebuffer; // immutable edge buffer for compaction
    eid_t bufcap; //max number of edges in ebuffer and immutable_ebuffer
    eid_t bufsize; //number of edges in ebuffer 

    vid_t nverts_per_blk; //number of vertices per block
    size_t logsize;
    size_t segsize;

    std::vector<std::vector<vid_t> > segs; //start vertex of each segment
        
public:
        
    DynamicGraph(metrics &_m, std::string _base_filename, vid_t _N, vid_t _nverts_per_blk, size_t buffersize = 0, size_t _logsize = 0, size_t _segsize = 0)
        : StaticGraph(_m, _base_filename, (bid_t)(_N / _nverts_per_blk + 1) ), nverts_per_blk(_nverts_per_blk), logsize(_logsize*1024*1024), segsize(_segsize*1024*1024){
        bufcap = (buffersize * 1024 * 1024) / (sizeof(vid_t)*2);
        bufsize = 0;
        ebuffer = (vid_t*) malloc(bufcap*sizeof(vid_t)*2);
        immutable_ebuffer = NULL;

        segs.resize(nblocks);
        for(bid_t p = 0; p < nblocks; p++){
            segs[p].push_back(0);
            segs[p].push_back(nverts_per_blk);
        }
        segs[nblocks-1][1] = N % nverts_per_blk;

        logstream(LOG_INFO) << "buffer capacity = " << bufcap << " edges(" << buffersize << "MB), " 
                            << "each block's logsize = " << _logsize << "MB, segsize = " << _segsize << "MB." << std::endl;
    }
        
    virtual ~DynamicGraph() {
        if(immutable_ebuffer != NULL) free(immutable_ebuffer);
        if(ebuffer != NULL) free(ebuffer);
    }

    bid_t getblock(vid_t v){
        bid_t p = (bid_t)(v/nverts_per_blk);
        assert(p < nblocks);
        return p;
    }

    bid_t getSegment(vid_t v, bid_t p){
        bid_t s = (bid_t)binarySearch(segs[p].data(), v-blocks[p], 0, segs[p].size()-1);
        return s;
    }

    void addEdge(vid_t s, vid_t d, bool isDel = 0){
        if(bufsize >= bufcap){
            immutable_ebuffer = ebuffer;
            flush();
        }
        ebuffer[2*bufsize] = s;
        ebuffer[2*bufsize+1] = d;
        bufsize++;
    }

    /***
    * Flush edge logs from memory buffer to disk log file
    * Incluing edge logs classification, each graph block equipment with a log file
    ***/
    void flush(){
        m.start_time("_flush_");
        
        //1. malloc logs
        m.start_time("_flush_1_malloc_logs");
        vid_t** blogs = (vid_t**)malloc(nblocks*sizeof(vid_t*));
        eid_t* nblogs = (eid_t*)malloc(nblocks*sizeof(eid_t));
        for(bid_t p = 0; p < nblocks; p++){
            blogs[p] = (vid_t*)malloc(bufcap*sizeof(vid_t)*2);
            nblogs[p] = 0;
        }
        m.stop_time("_flush_1_malloc_logs");

        //2. edge log classification by block
        m.start_time("_flush_2_log_classification");
        for(eid_t e = 0; e < bufcap; e++){
            bid_t p = getblock(immutable_ebuffer[2*e]);
            assert(p < nblocks);
            blogs[p][2*nblogs[p]] = immutable_ebuffer[2*e];
            blogs[p][2*nblogs[p]+1] = immutable_ebuffer[2*e+1];
            nblogs[p]++;
        }
        m.stop_time("_flush_2_log_classification");

        //3. write edge logs for each block
        m.start_time("_flush_3_write_logs");
        for(bid_t p = 0; p < nblocks; p++){
            if(nblogs[p]>0){
                std::string logfile = blockname( base_filename, p ) + ".log";

                appendfile(logfile, blogs[p], nblogs[p] * 2 * sizeof(vid_t));

                size_t fsize = filesize(logfile);
                // logstream(LOG_WARNING) << logfile << ", filesize = " << fsize/(1024*1024) << "MB, nlogs[" << p << "] = " << nblogs[p] << std::endl;
                if(fsize >= logsize){ 
                    compaction(p);
                    remove(logfile.c_str());
                }
            }
            free(blogs[p]);
        }
        m.stop_time("_flush_3_write_logs");

        //4. free space
        m.start_time("_flush_4_free_logs");
        free(nblogs);
        free(blogs);
        bufsize = 0;
        m.stop_time("_flush_4_free_logs");

        m.stop_time("_flush_");
    }

    void compaction(bid_t p){
        m.start_time("_compaction_");
        
        //1. Load the logs of block_p
        m.start_time("_compaction_1_loadLog");
        std::string logfile = blockname( base_filename, p ) + ".log";
        vid_t* logs;
        eid_t nlogs = readfile(logfile, &logs) / (sizeof(vid_t)*2);
        assert(nlogs > 0);
        m.stop_time("_compaction_1_loadLog");

        //2. compute new degree for each vertex
        m.start_time("_compaction_2_computeDegree");
        eid_t* newdeg = (eid_t*)malloc(nverts_per_blk*sizeof(eid_t));
        memset(newdeg, 0, nverts_per_blk*sizeof(eid_t));
        for(eid_t e = 0; e < nlogs; e++){
            vid_t v = logs[2*e]-blocks[p];
            assert(v >= 0 && v < nverts_per_blk);
            newdeg[v]++;
        }
        m.stop_time("_compaction_2_computeDegree");

        bid_t nsegs = segs[p].size()-1;
        bid_t nsplits = 0;
        for(bid_t ss = 0; ss < nsegs; ss++){
            bid_t s = ss + nsplits;
            eid_t nupdates = 0;
            for(vid_t v = segs[p][s]; v < segs[p][s+1]; v++){
                if(v >= nverts_per_blk) logstream(LOG_WARNING) << v << std::endl;
                assert(v >= 0 && v < nverts_per_blk);
                nupdates += newdeg[v];
            }
            if(nupdates > 0){
                //3. Load the CSR from disk
                m.start_time("_compaction_3_loadSubGraph");
                vid_t nverts, *csr = NULL;
                eid_t nedges, *beg_pos = NULL;
                loadSubGraphSegment(p, s, beg_pos, csr, &nverts, &nedges);
                // logstream(LOG_DEBUG) << "loaded " << nverts << " vertices and " << nedges << " edges of subgraph_" << p << ": [" << blocks[p] << ", " << blocks[p+1] << ")" << std::endl;
                m.stop_time("_compaction_3_loadSubGraph");

                //4. Allocate new space
                m.start_time("_compaction_4_mallocNewCSR");
                // logstream(LOG_DEBUG) << "p = " << p << ", s = " << s << ", nedges = " << nedges << ", nupdates = " << nupdates << std::endl;
                nedges += nupdates;
                vid_t * newcsr = (vid_t*) malloc(nedges*sizeof(vid_t));
                eid_t * newbeg_pos = (eid_t*) malloc((nverts+1)*sizeof(eid_t));
                m.stop_time("_compaction_4_mallocNewCSR");

                //5. Compute new begpos
                m.start_time("_compaction_5_compBeg");
                eid_t begoff = 0;
                newbeg_pos[0] = 0;
                for(vid_t v = 0; v < nverts; v++){
                    if(newdeg[v+segs[p][s]] > 0)   begoff += newdeg[v+segs[p][s]];
                    newbeg_pos[v+1] = beg_pos[v+1] + begoff;
                }
                // logstream(LOG_DEBUG) << "nverts = " << nverts << ", newbeg_pos[nverts] = " << newbeg_pos[nverts] << ", nedges = " << nedges << std::endl;
                assert(newbeg_pos[nverts] == nedges);
                m.stop_time("_compaction_5_compBeg");

                //6. copy CSR
                m.start_time("_compaction_6_copyCSR");
                for(vid_t v = 0; v < nverts; v++){
                    if(beg_pos[v+1]-beg_pos[v] > 0){
                        memcpy(newcsr+newbeg_pos[v], csr+beg_pos[v], (beg_pos[v+1]-beg_pos[v])*sizeof(vid_t));
                    }
                }
                m.stop_time("_compaction_6_copyCSR");

                //7. write edge log to CSR
                m.start_time("_compaction_7_mergeLog2CSR");
                for(eid_t e = 0; e < nlogs; e++){
                    vid_t v = logs[2*e] - blocks[p];
                    if(v >= segs[p][s] && v < segs[p][s+1]){
                        assert(newdeg[v] > 0);
                        eid_t pos = newbeg_pos[v-segs[p][s]+1] - newdeg[v];
                        newcsr[pos] = logs[2*e+1];
                        newdeg[v]--;
                    }
                }
                m.stop_time("_compaction_7_mergeLog2CSR");

                //8. Rewrite the CSR to disk
                if( (size_t)(nedges*sizeof(vid_t)) >= segsize){
                    m.start_time("_compaction_8_splitSubGraph");
                    splitSubGraphSegment(p, s, newcsr, nedges, newbeg_pos, nverts);
                    nsplits++;
                    m.stop_time("_compaction_8_splitSubGraph");
                }else{
                    m.start_time("_compaction_8_writeSubGraph");
                    writeSubGraphSegment(p, s, newcsr, nedges, newbeg_pos, nverts);
                    m.stop_time("_compaction_8_writeSubGraph");
                    // logstream(LOG_WARNING) << "After compaction block_" << p << "_" << s <<", we have " << nverts << " vertices and " << nedges << " edges." << std::endl;
                }

                //9. Free memory
                m.start_time("_compaction_9_free");
                if(beg_pos != NULL) free(beg_pos);
                if(csr != NULL) free(csr);
                csr = (vid_t*)newcsr;
                beg_pos = (eid_t*)newbeg_pos;
                if(beg_pos != NULL) free(beg_pos);
                if(csr != NULL) free(csr);
                m.stop_time("_compaction_9_free");
            }
        }
        if(newdeg != NULL) free(newdeg);
        if(logs != NULL) free(logs);
        
        m.stop_time("_compaction_");
    }

    void loadSubGraphSegment(bid_t p, bid_t s, eid_t * &beg_pos, vid_t * &csr, vid_t *nverts, eid_t *nedges){
        std::string segname = segmentname(base_filename, p, segs[p][s]);
        *nverts = segs[p][s+1] - segs[p][s];
        loadBegpos(segname, beg_pos, *nverts);
        *nedges = beg_pos[*nverts] - beg_pos[0];
        loadCSR(segname, csr, *nedges);
    }

    void writeSubGraphSegment(bid_t p, bid_t s, vid_t* csr, eid_t nedges, eid_t* beg_pos, vid_t nverts){
        std::string segname = segmentname(base_filename, p, segs[p][s]);
        std::string beg_posname = segname + ".beg_pos";
        writefile(beg_posname, beg_pos, (size_t)(nverts+1)*sizeof(eid_t));
        std::string csrname = segname + ".csr";
        writefile(csrname, csr, (size_t)nedges*sizeof(vid_t));
    }

    void splitSubGraphSegment(bid_t p, bid_t s, vid_t* csr, eid_t nedges, eid_t* beg_pos, vid_t nverts){
        vid_t nverts1 = (vid_t)binarySearch(beg_pos, nedges/2, 0, nverts);
        eid_t nedges1 = beg_pos[nverts1];

        auto it = segs[p].begin() + s + 1;
        segs[p].insert(it, segs[p][s]+nverts1);

        writeSubGraphSegment(p, s, csr, nedges1, beg_pos, nverts1);
        for(vid_t v = nverts1; v <= nverts; v++)    beg_pos[v] -= nedges1;
        writeSubGraphSegment(p, s+1, csr+nedges1, nedges-nedges1, beg_pos+nverts1, nverts-nverts1);

        logstream(LOG_DEBUG) << "Split subgraph " << p << "_" << s << ": [" << blocks[p]+segs[p][s] << ", " << blocks[p]+segs[p][s+2] << ") into :" << std::endl;
        logstream(LOG_INFO) << " " << p << "_" << s << ": [" << blocks[p]+segs[p][s] << ", " << blocks[p]+segs[p][s+1] << "), #edges = " << nedges1 << std::endl;
        logstream(LOG_INFO) << " " << p << "_" << s+1 << ": [" << blocks[p]+segs[p][s+1] << ", " << blocks[p]+segs[p][s+2] << "), #edges = " << nedges-nedges1 << std::endl;
    }
    
    void loadSubGraph(bid_t p, eid_t * &beg_pos, vid_t * &csr, vid_t *nverts, eid_t *nedges){

        //3. Search logs of memory edge buffer
        /*
        vid_t* logs = (vid_t*)realloc(logs, (nlogs+bufsize)*sizeof(vid_t)*2);
        for(eid_t e = 0; e < bufsize; e++){
            if(getblock(ebuffer[2*e]) == p){
                logs[2*nlogs] = ebuffer[2*e];
                logs[2*nlogs+1] = ebuffer[2*e+1];
                nlogs++;
            }
        }
        */
    }

    std::vector<vid_t> getNeighbors(vid_t v){
        if(v >= N){
            logstream(LOG_ERROR) << "Invalid vertex id : " << v << " >= " << N << std::endl;
            std::vector<vid_t> neighbors;
            return neighbors;
        }

        std::vector<vid_t> neighbors = getNeighborsInSegment(v);
        logstream(LOG_DEBUG) << "After loadBlock, # of neighbors = " << neighbors.size() << std::endl;

        // load and search logs of block_p
        bid_t p = getblock(v);
        std::string logfile = blockname( base_filename, p ) + ".log";
        vid_t* logs;
        eid_t nlogs = readfile(logfile, &logs) / (sizeof(vid_t)*2);
        for(eid_t e = 0; e < nlogs; e++){
            if(logs[2*e] == v){
                neighbors.push_back(logs[2*e+1]);
            }
        }
        if(logs != nullptr) free(logs);
        logstream(LOG_DEBUG) << "After loadLog, # of neighbors = " << neighbors.size() << std::endl;

        // search logs of memory edge buffer
        for(eid_t e = 0; e < bufsize; e++){
            if(ebuffer[2*e] == v){
                neighbors.push_back(ebuffer[2*e+1]);
            }
        }
        logstream(LOG_DEBUG) << "After searchBuffer, # of neighbors = " << neighbors.size() << std::endl;

        return neighbors;
    }

    std::vector<vid_t> getNeighborsInSegment(vid_t v){
        std::vector<vid_t> neighbors;

        bid_t p = getblock(v);
        bid_t s = getSegment(v, p);
        std::string segname = segmentname(base_filename, p, segs[p][s]);

        // logstream(LOG_WARNING) << v << " " << segname << std::endl;

        eid_t *beg_pos;// = (eid_t*)malloc(2*sizeof(eid_t));
        vid_t off = v - blocks[p];
        loadBegpos(segname, beg_pos, 1, off);

        eid_t nedges = beg_pos[1] - beg_pos[0];
        if(nedges > 0){
            // logstream(LOG_WARNING) << nedges << std::endl;
            vid_t *csr;// = (vid_t*)malloc(nedges*sizeof(vid_t));
            loadCSR(segname, csr, nedges, beg_pos[0]);
            for(eid_t i = 0; i < nedges; i++){
                neighbors.push_back(csr[i]);
            }
            if(csr != NULL) free(csr);
        }
        if(beg_pos != NULL) free(beg_pos);

        return neighbors;
    }

    void writeSegmentRange(){
        std::string segrangename = segmentrangename(base_filename);
        std::ofstream brf(segrangename.c_str());
        for( bid_t p = 0; p < nblocks; p++ ){
            brf << "Block " << p << " : [" << blocks[p] << " " << blocks[p+1] << ")" << std::endl;
            for(bid_t s = 0; s < segs[p].size(); s++)
                brf << segs[p][s] << " " ;
            brf << std::endl;
        }
        brf.close();
    }


};

#endif