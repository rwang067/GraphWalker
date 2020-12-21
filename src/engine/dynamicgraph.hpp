
#ifndef DEF_DYNAMIC_GRAPH
#define DEF_DYNAMIC_GRAPH

#include <algorithm>

#include "engine/staticgraph.hpp"

class DynamicGraph : public StaticGraph {
public:     
    vid_t nverts_per_blk; //number of vertices per block
    uint8_t nbits_nverts_per_blk;

    vid_t *ebuffer; //edge buffer
    vid_t *immutable_ebuffer; // immutable edge buffer for compaction
    eid_t bufcap; //max number of edges in ebuffer and immutable_ebuffer
    eid_t bufsize; //number of edges in ebuffer 

    bid_t ngroups; // number of log groups 
    vid_t nverts_per_grp; //number of vertices per log group
    uint8_t nbits_nverts_per_grp;

    size_t logsize;
    size_t segsize;

    std::vector<std::vector<vid_t> > segs; //start vertex of each segment

    bid_t ngrps_per_blk;
    eid_t logcap;
    eid_t* nglogs;
    vid_t** glogs;
        
public:
        
    DynamicGraph(metrics &_m, std::string _base_filename, vid_t _N, vid_t _nverts_per_blk, vid_t _nverts_per_grp, 
                size_t buffersize = 0, size_t _logsize = 0, size_t _segsize = 0)
        : StaticGraph(_m, _base_filename, (bid_t)(_N / _nverts_per_blk + 1) ), 
        nverts_per_blk(_nverts_per_blk), nbits_nverts_per_blk(log(_nverts_per_blk)/log(2)), 
        ebuffer(NULL), immutable_ebuffer(NULL), bufcap((buffersize * 1024 * 1024) / (sizeof(vid_t)*2)), bufsize(0), 
        ngroups((bid_t)(_N / _nverts_per_grp + 1)), nverts_per_grp(_nverts_per_grp), nbits_nverts_per_grp(log(_nverts_per_grp)/log(2)),
        logsize(_logsize*1024), segsize(_segsize*1024*1024){

        ebuffer = (vid_t*) malloc(bufcap*sizeof(vid_t)*2);

        segs.resize(nblocks);
        for(bid_t p = 0; p < nblocks; p++){
            segs[p].push_back(0);
            segs[p].push_back(nverts_per_blk);
        }
        segs[nblocks-1][1] = N % nverts_per_blk;
        
        //1. malloc logs
        ngrps_per_blk = nverts_per_blk / nverts_per_grp;
        m.start_time("_flush_1_malloc_logs");
        logcap = ( logsize / sizeof(vid_t) /2) / 4;
        logstream(LOG_WARNING) << "logcap = " << logcap << " edges. " << std::endl;
        nglogs = (eid_t*)malloc(ngroups*sizeof(eid_t*));
        glogs = (vid_t**)malloc(ngroups*sizeof(vid_t**));
        for(bid_t g = 0; g < ngroups; g++){
            nglogs[g] = 0;
            glogs[g] = (vid_t*)malloc(logcap*sizeof(vid_t)*2);
        }
        m.stop_time("_flush_1_malloc_logs");

        logstream(LOG_INFO) << "buffer capacity = " << bufcap << " edges(" << buffersize << "MB)\n" 
                            << "each block's logsize = " << _logsize << "KB, segsize = " << _segsize << "MB.\n" 
                            << "nverts_per_grp = " << nverts_per_grp << ", ngrps_per_blk = " << ngrps_per_blk << ",\n"
                            << "nblocks = " << nblocks << ", ngroups = " << ngroups << ".\n"
                            << std::endl;
    }
        
    virtual ~DynamicGraph() {
        if(immutable_ebuffer != NULL) free(immutable_ebuffer);
        if(ebuffer != NULL) free(ebuffer);

        //4. free space
        m.start_time("_flush_4_free_logs");
        for(bid_t g = 0; g < ngroups; g++){
            free(glogs[g]);
        }
        free(glogs);
        free(nglogs);
        m.stop_time("_flush_4_free_logs");
    }

    inline bid_t getblock(vid_t v){
        bid_t p = (bid_t)(v/nverts_per_blk);
        assert(p < nblocks);
        return p;
    }

    bid_t getSegment(vid_t v, bid_t p){
        bid_t s = 0;
        if(segs[p].size() > 1) s = (bid_t)binarySearch(segs[p].data(), v-blocks[p], 0, segs[p].size()-1);
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
    void flush1(){
        m.start_time("_flush_");
        
        //1. malloc logs
        m.start_time("_flush_1_malloc_logs");
        bid_t* nsegs = (bid_t*)malloc(nblocks*sizeof(bid_t));
        eid_t** nseglogs = (eid_t**)malloc(nblocks*sizeof(eid_t*));
        vid_t*** seglogs = (vid_t***)malloc(nblocks*sizeof(vid_t**));
        for(bid_t p = 0; p < nblocks; p++){
            nsegs[p] = segs[p].size()-1;
            nseglogs[p] = (eid_t*)malloc(nsegs[p]*sizeof(eid_t));
            seglogs[p] = (vid_t**)malloc(nsegs[p]*sizeof(vid_t*));
            for(bid_t s = 0; s < nsegs[p]; s++){
                nseglogs[p][s] = 0;
                seglogs[p][s] = (vid_t*)malloc(bufcap*sizeof(vid_t)*2);
            }
        }
        m.stop_time("_flush_1_malloc_logs");

        //2. edge log classification by block
        m.start_time("_flush_2_log_classification");
        for(eid_t e = 0; e < bufcap; e++){
            vid_t v = immutable_ebuffer[2*e];
            // bid_t p = (bid_t)(v/nverts_per_blk);
            bid_t p = (bid_t)(v >> nbits_nverts_per_blk);
            bid_t s = 0;
            if(nsegs[p] > 1){
                // s = (bid_t)binarySearch(segs[p].data(), v - blocks[p], 0, nsegs[p]);
                vid_t val = v - blocks[p];
                vid_t* array = segs[p].data();
                while(val >= array[s+1])    s++;
            }
            eid_t i = 2*nseglogs[p][s];
            seglogs[p][s][i] = v;
            seglogs[p][s][i+1] = immutable_ebuffer[2*e+1];
            nseglogs[p][s]++;
        }
        m.stop_time("_flush_2_log_classification");

        //3. write edge logs for each block
        m.start_time("_flush_3_write_logs");
        for(bid_t p = 0; p < nblocks; p++){
            bid_t nsplits = 0;
            for(bid_t s = 0; s < nsegs[p]; s++){
                if(nseglogs[p][s]>0){
                    bid_t ss = s + nsplits;
                    std::string logfile = segmentname(base_filename, p, segs[p][ss]) + ".log";
                    appendfile(logfile, seglogs[p][s], nseglogs[p][s] * 2 * sizeof(vid_t));
                    size_t fsize = filesize(logfile);
                    // logstream(LOG_WARNING) << logfile << ", filesize = " << fsize/(1024*1024) << "MB, nlogs[" << p << "] = " << nblogs[p] << std::endl;
                    if(fsize >= logsize){ 
                        nsplits += compaction(p, ss);
                        remove(logfile.c_str());
                    }
                }
            }
        }
        m.stop_time("_flush_3_write_logs");

        //4. free space
        m.start_time("_flush_4_free_logs");
        for(bid_t p = 0; p < nblocks; p++){
            for(bid_t s = 0; s < nsegs[p]; s++){
                free(seglogs[p][s]);
            }
            free(seglogs[p]);
            free(nseglogs[p]);
        }
        free(seglogs);
        free(nseglogs);
        free(nsegs);
        bufsize = 0;
        m.stop_time("_flush_4_free_logs");

        m.stop_time("_flush_");
    }

    /***
    * Flush edge logs from memory buffer to disk log file
    * Incluing edge logs classification, each graph block equipment with a log file
    ***/
    void flush(){
        m.start_time("_2_flush_");

        //2. edge log classification by block
        m.start_time("_3_flush_2_classifyLog");
        for(eid_t e = 0; e < bufcap; e++){
            vid_t v = immutable_ebuffer[2*e];
            bid_t g = (bid_t)(v >> nbits_nverts_per_grp);
            eid_t i = 2*nglogs[g];
            glogs[g][i] = v;
            glogs[g][i+1] = immutable_ebuffer[2*e+1];
            nglogs[g]++;
            if(nglogs[g] == logcap){
                writeLog(g);
            }
        }
        m.stop_time("_3_flush_2_classifyLog");

        bufsize = 0;

        m.stop_time("_2_flush_");
    }

    void writeLog(bid_t g){
        m.start_time("_4_flush_3_writeLogs");
        if(nglogs[g] > 0){
            std::string logfile = logname(base_filename, g);
            appendfile(logfile, glogs[g], nglogs[g] * 2 * sizeof(vid_t));
            size_t fsize = filesize(logfile);
            if(fsize >= logsize){ 
                bid_t p = g / ngrps_per_blk;
                bid_t s = 0;//(bid_t)binarySearch(segs[p].data(), g*nverts_per_grp - blocks[p], 0, segs[p].size()-1);
                compaction(p, s);
                for(bid_t g1 = p*ngrps_per_blk; g1 < (p+1)*ngrps_per_blk; g1++){
                    // logstream(LOG_WARNING) << g1 << " " << g << " " << p << " " << s << std::endl;
                    remove(logname(base_filename, g1).c_str());
                }
            }
            nglogs[g] = 0;
        }
        m.stop_time("_4_flush_3_writeLogs");
    }

    void writeLogfiles(){
        for(bid_t g = 0; g < ngroups; g++){
            writeLog(g);
        }
    }

    eid_t compaction(bid_t p, bid_t s){
        m.start_time("_5_compaction_");
        
        m.start_time("_compaction_1_loadSubGraph");
        vid_t nverts = 0, *csr = NULL;
        eid_t nedges = 0, *beg_pos = NULL;
        loadSubGraph(p, s, beg_pos, csr, &nverts, &nedges);
        m.stop_time("_compaction_1_loadSubGraph");

        //8. Rewrite the CSR to disk
        eid_t isSplit = 0;
        if( (size_t)(nedges*sizeof(vid_t)) >= segsize){
            m.start_time("_compaction_2_splitSubGraph");
            splitSubGraphSegment(p, s, csr, nedges, beg_pos, nverts);
            isSplit = 1;
            m.stop_time("_compaction_2_splitSubGraph");
        }else{
            m.start_time("_compaction_2_writeSubGraph");
            writeSubGraphCSR(p, s, csr, nedges, beg_pos, nverts);
            m.stop_time("_compaction_2_writeSubGraph");
            // logstream(LOG_WARNING) << "After compaction block_" << p << "_" << s <<", we have " << nverts << " vertices and " << nedges << " edges." << std::endl;
        }

        //9. Free memory
        m.start_time("_compaction_3_free");
        if(beg_pos != NULL) free(beg_pos);
        if(csr != NULL) free(csr);
        m.stop_time("_compaction_3_free");

        
        m.stop_time("_5_compaction_");

        return isSplit;
    }

    
    void loadSubGraph(bid_t p, bid_t s, eid_t * &beg_pos, vid_t * &csr, vid_t *nverts, eid_t *nedges){

        //3. Load the CSR from disk
        m.start_time("_load_1_loadCSR");
        *nverts = segs[p][s+1] - segs[p][s];
        loadSubGraphCSR(p, s, beg_pos, csr, *nverts, nedges);
        m.stop_time("_load_1_loadCSR");
        // logstream(LOG_WARNING) << p << " " << *nverts << " " << *nedges << std::endl;

        //1. Load the logs of block_p
        m.start_time("_load_2_loadLog");
        vid_t** logs = (vid_t**)malloc(ngrps_per_blk*sizeof(vid_t*));
        eid_t* nlogs = (eid_t*)malloc(ngrps_per_blk*sizeof(eid_t));
        for(bid_t g = 0; g < ngrps_per_blk; g++){
            std::string logfile = logname(base_filename, p*ngrps_per_blk+g);
            nlogs[g] = readfile(logfile, &logs[g]) / (sizeof(vid_t)*2);
        }
        m.stop_time("_load_2_loadLog");

        //2. compute new degree for each vertex
        m.start_time("_load_3_computeDegree");
        vid_t stv = blocks[p] + segs[p][s];
        eid_t* newdeg = (eid_t*)malloc((*nverts)*sizeof(eid_t));
        memset(newdeg, 0, (*nverts)*sizeof(eid_t));
        for(bid_t g = 0; g < ngrps_per_blk; g++){
            for(eid_t e = 0; e < nlogs[g]; e++){
                logs[g][2*e] -= stv;
                newdeg[logs[g][2*e]]++;
            }
            *nedges += nlogs[g];
            // logstream(LOG_WARNING) << g << " " << nlogs[g] << " " << *nedges << std::endl;
        }
        m.stop_time("_load_3_computeDegree");
        // logstream(LOG_DEBUG) << p << " " << *nverts << " " << *nedges << std::endl;

        //5. Compute new begpos
        m.start_time("_load_4_compBeg");
        vid_t * newcsr = (vid_t*) malloc((*nedges)*sizeof(vid_t));
        eid_t * newbeg_pos = (eid_t*) malloc((*nverts+1)*sizeof(eid_t));
        eid_t begoff = 0;
        newbeg_pos[0] = 0;
        for(vid_t v = 0; v < *nverts; v++){
            if(newdeg[v] > 0)   begoff += newdeg[v];
            newbeg_pos[v+1] = beg_pos[v+1] + begoff;
        }
        assert(newbeg_pos[*nverts] == *nedges);
        m.stop_time("_load_4_compBeg");

        //6. copy CSR
        m.start_time("_load_5_copyCSR");
        for(vid_t v = 0; v < *nverts; v++){
            if(beg_pos[v+1]-beg_pos[v] > 0){
                vid_t u = v;
                while(newdeg[v]==0 && v < *nverts-1) v++;
                memcpy(newcsr+newbeg_pos[u], csr+beg_pos[u], (beg_pos[v+1]-beg_pos[u])*sizeof(vid_t));
            }
        }
        m.stop_time("_load_5_copyCSR");

        //7. write edge log to CSR
        m.start_time("_load_6_mergeLog2CSR");
        for(bid_t g = 0; g < ngrps_per_blk; g++){
            for(eid_t e = 0; e < nlogs[g]; e++){
                vid_t v = logs[g][2*e];
                eid_t pos = newbeg_pos[v+1] - newdeg[v];
                newcsr[pos] = logs[g][2*e+1];
                newdeg[v]--;
            }
        }
        m.stop_time("_load_6_mergeLog2CSR");

        
        m.start_time("_load_7_free");
        for(bid_t g = 0; g < ngrps_per_blk; g++){
            if(logs[g] != NULL) free(logs[g]);
        }
        if(logs != NULL) free(logs);
        if(nlogs != NULL) free(nlogs);
        if(beg_pos != NULL) free(beg_pos);
        if(csr != NULL) free(csr);
        csr = (vid_t*)newcsr;
        beg_pos = (eid_t*)newbeg_pos;
        m.stop_time("_load_7_free");
    }

    void loadSubGraphCSR(bid_t p, bid_t s, eid_t * &beg_pos, vid_t * &csr, vid_t nverts, eid_t *nedges){
        std::string segname = segmentname(base_filename, p, segs[p][s]);
        loadBegpos(segname, beg_pos, nverts);
        *nedges = beg_pos[nverts] - beg_pos[0];
        loadCSR(segname, csr, *nedges);
    }

    void writeSubGraphCSR(bid_t p, bid_t s, vid_t* csr, eid_t nedges, eid_t* beg_pos, vid_t nverts){
        std::string segname = segmentname(base_filename, p, segs[p][s]);
        std::string beg_posname = segname + ".beg_pos";
        writefile(beg_posname, beg_pos, (size_t)(nverts+1)*sizeof(eid_t));
        std::string csrname = segname + ".csr";
        writefile(csrname, csr, (size_t)nedges*sizeof(vid_t));
    }

    void splitSubGraphSegment(bid_t p, bid_t s, vid_t* csr, eid_t nedges, eid_t* beg_pos, vid_t nverts){
        
        bid_t nverts1 = (vid_t)binarySearch(beg_pos, nedges/2, 0, nverts);
        eid_t nedges1 = beg_pos[nverts1];

        auto it = segs[p].begin() + s + 1;
        segs[p].insert(it, segs[p][s]+nverts1);

        writeSubGraphCSR(p, s, csr, nedges1, beg_pos, nverts1);
        for(vid_t v = nverts1; v <= nverts; v++)    beg_pos[v] -= nedges1;
        writeSubGraphCSR(p, s+1, csr+nedges1, nedges-nedges1, beg_pos+nverts1, nverts-nverts1);

        // logstream(LOG_DEBUG) << "Split subgraph " << p << "_" << s << ": [" << blocks[p]+segs[p][s] << ", " << blocks[p]+segs[p][s+2] << ") into :" << std::endl;
        // logstream(LOG_INFO) << " " << p << "_" << s << ": [" << blocks[p]+segs[p][s] << ", " << blocks[p]+segs[p][s+1] << "), #edges = " << nedges1 << std::endl;
        // logstream(LOG_INFO) << " " << p << "_" << s+1 << ": [" << blocks[p]+segs[p][s+1] << ", " << blocks[p]+segs[p][s+2] << "), #edges = " << nedges-nedges1 << std::endl;
    }

    void splitSubGraphSegment1(bid_t p, bid_t s, vid_t* csr, eid_t nedges, eid_t* beg_pos, vid_t nverts){
        vid_t nverts1 = (vid_t)binarySearch(beg_pos, nedges/2, 0, nverts);
        eid_t nedges1 = beg_pos[nverts1];

        auto it = segs[p].begin() + s + 1;
        segs[p].insert(it, segs[p][s]+nverts1);

        writeSubGraphCSR(p, s, csr, nedges1, beg_pos, nverts1);
        for(vid_t v = nverts1; v <= nverts; v++)    beg_pos[v] -= nedges1;
        writeSubGraphCSR(p, s+1, csr+nedges1, nedges-nedges1, beg_pos+nverts1, nverts-nverts1);

        // logstream(LOG_DEBUG) << "Split subgraph " << p << "_" << s << ": [" << blocks[p]+segs[p][s] << ", " << blocks[p]+segs[p][s+2] << ") into :" << std::endl;
        // logstream(LOG_INFO) << " " << p << "_" << s << ": [" << blocks[p]+segs[p][s] << ", " << blocks[p]+segs[p][s+1] << "), #edges = " << nedges1 << std::endl;
        // logstream(LOG_INFO) << " " << p << "_" << s+1 << ": [" << blocks[p]+segs[p][s+1] << ", " << blocks[p]+segs[p][s+2] << "), #edges = " << nedges-nedges1 << std::endl;
    }

    std::vector<vid_t> getNeighbors(vid_t v){
        if(v >= N){
            logstream(LOG_ERROR) << "Invalid vertex id : " << v << " >= " << N << std::endl;
            std::vector<vid_t> neighbors;
            return neighbors;
        }

        bid_t p = getblock(v);
        bid_t s = getSegment(v, p);
        // logstream(LOG_DEBUG) << "v = " << v << ", p = " << p << ", s = " << s << ", segs[p].size() = " << segs[p].size() << std::endl;

        m.start_time("test_searchNeighbors_1_InSegmentCSR");
        std::vector<vid_t> neighbors = getNeighborsInSegment(v, p, s);
        // logstream(LOG_DEBUG) << "After loadBlock, # of neighbors = " << neighbors.size() << std::endl;
        m.stop_time("test_searchNeighbors_1_InSegmentCSR");

        // load and search logs of block_p
        m.start_time("test_searchNeighbors_2_InLogfile");
        m.start_time("test_searchNeighbors_2_InLogfile_1_readfile");
        // std::string logfile = segmentname(base_filename, p, segs[p][s]) + ".log";
        bid_t g = v >> nbits_nverts_per_grp;
        std::string logfile = logname(base_filename, g);
        vid_t* logs;
        eid_t nlogs = readfile(logfile, &logs) / (sizeof(vid_t)*2);
        m.stop_time("test_searchNeighbors_2_InLogfile_1_readfile");
        m.start_time("test_searchNeighbors_2_InLogfile_2_searchInfile");
        for(eid_t e = 0; e < nlogs; e++){
            if(logs[2*e] == v){
                neighbors.push_back(logs[2*e+1]);
            }
        }
        m.stop_time("test_searchNeighbors_2_InLogfile_2_searchInfile");
        m.start_time("test_searchNeighbors_2_InLogfile_3_freeLog");
        if(logs != nullptr) free(logs);
        // logstream(LOG_DEBUG) << "After loadLog, # of neighbors = " << neighbors.size() << std::endl;
        m.stop_time("test_searchNeighbors_2_InLogfile_3_freeLog");
        m.stop_time("test_searchNeighbors_2_InLogfile");

        // search logs of memory edge buffer
        m.start_time("test_searchNeighbors_3_InMembuf");
        for(eid_t e = 0; e < bufsize; e++){
            if(ebuffer[2*e] == v){
                neighbors.push_back(ebuffer[2*e+1]);
            }
        }
        // logstream(LOG_DEBUG) << "After searchBuffer, # of neighbors = " << neighbors.size() << std::endl;
        m.stop_time("test_searchNeighbors_3_InMembuf");

        return neighbors;
    }

    std::vector<vid_t> getNeighborsInSegment(vid_t v, bid_t p, bid_t s){
        std::vector<vid_t> neighbors;

        std::string segname = segmentname(base_filename, p, segs[p][s]);
        // logstream(LOG_WARNING) << v << " " << segname << std::endl;
        eid_t *beg_pos;// = (eid_t*)malloc(2*sizeof(eid_t));
        vid_t off = v - blocks[p] - segs[p][s];
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