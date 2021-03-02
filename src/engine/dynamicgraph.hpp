
#ifndef DEF_DYNAMIC_GRAPH
#define DEF_DYNAMIC_GRAPH

#include <algorithm>

#include "engine/staticgraph.hpp"

class DynamicGraph : public StaticGraph {
public:
    /** dynamic vertices management **/
    bid_t ngroups; //number of log groups 
    vid_t nverts_per_grp; //number of vertices per log group
    uint8_t nbits_nverts_per_grp; //number of bits of nverts_per_grp

    /** memory buffer management **/
    vid_t *ebuffer; //edge buffer
    eid_t bufsize; //capcity (Bytes) of ebuffer 
    eid_t bufcap; //max number of edges in ebuffer
    eid_t nebuf; //number of edges in ebuffer 
    std::vector<BitMap> membitmaps; //bitmap to indicate whether a vertex has edges in memory buffer

    /** disk edge logs management **/
    vid_t *elogs; //memory group log buffer
    size_t logsize; //capcity (Bytes) of disk log file
    eid_t logcap; //max number of logs in a memory group log buffer
    eid_t nelogs; //number of logs in a memory group log buffer
    std::vector<BitMap> logbitmaps; //bitmap to indicate whether a vertex has edges in log file
        
public:
        
    DynamicGraph(metrics &_m, std::string _base_filename,
        vid_t _nverts_per_grp = 0, size_t _bufsize = 0, size_t _logsize = 0)
        : StaticGraph(_m, _base_filename, 1, 0, 0), 
        ngroups(0), nverts_per_grp(_nverts_per_grp), nbits_nverts_per_grp(log(_nverts_per_grp)/log(2)),
        ebuffer(NULL), bufsize(_bufsize*1024*1024), bufcap(bufsize / (sizeof(vid_t)*2)), nebuf(0), 
        elogs(NULL), logsize(_logsize*1024*1024), logcap(logsize / (sizeof(vid_t)*2) / 4), nelogs(0){


        /** memory buffer management **/
        if(bufsize==0){ 
            bufsize = sizeof(vid_t)*2;
            bufcap = 1;
        }
        ebuffer = (vid_t*) malloc(bufsize);
        elogs = (vid_t*) malloc(logsize);
        
        /** edge logs mangement **/
        addLogGroup();

        logstream(LOG_INFO) << "number of log groups = " << ngroups << ", nverts_per_grp = " << nverts_per_grp << ", nbits_nverts_per_grp = " << (int)nbits_nverts_per_grp << ".\n" 
                            << "edge buffer capacity = " << bufcap << " edges(" << _bufsize << "MB).\n" 
                            << "edge log capacity = " << logcap << " edges, logsize = " << _logsize << "MB.\n" 
                            << std::endl;
    }
        
    virtual ~DynamicGraph() {
        m.start_time("destroyGraph");
        if(ebuffer != NULL) free(ebuffer);
        if(elogs != NULL) free(elogs);
        m.stop_time("destroyGraph");
    }

    inline void addLogGroup(){
        m.start_time("addLogGroup");
        BitMap bitmap1 = BitMap(nverts_per_grp/8);
        BitMap bitmap2 = BitMap(nverts_per_grp/8);
        membitmaps.push_back(bitmap1);
        logbitmaps.push_back(bitmap2);
        ngroups++;
        m.stop_time("addLogGroup");
    }
    
    inline void addVertex(){
        N++;
        if(N % nverts_per_grp == 0) addLogGroup();
    }
    
    void addEdge(vid_t s, vid_t t, bool isDel = 0){
        if(s == N) addVertex();
        if(t == N) addVertex();
        if(!(s <= N && t <= N)){
            logstream(LOG_ERROR) << "Wrong vertex id: " << s << " " << t << " " << N << std::endl;
            return ;
        }
        
        if(nebuf >= bufcap){
            flush();
        }
        ebuffer[2*nebuf] = s;
        ebuffer[2*nebuf+1] = t;
        nebuf++;

        bid_t g = (bid_t)(s >> nbits_nverts_per_grp);
        membitmaps[g].bitmapSet(s & (nverts_per_grp-1));
    }

    /***
    * Flush edge logs from memory buffer to disk log file
    * Incluing edge logs classification, each graph block equipment with a log file
    ***/
    void flush(){
        m.start_time("_2_flush_");
        writeLog();
        for(bid_t g = 0; g < ngroups; g++)
            membitmaps[g].bitmapReset();
        m.stop_time("_2_flush_");
    }

    /***
    * Write logs of a log group to log file
    ***/
    void writeLog(){
        m.start_time("_4_flush_3_writeLogs");

        for(eid_t e = 0; e < nebuf; e++){
            vid_t s = ebuffer[2*e];
            bid_t g = (bid_t)(s >> nbits_nverts_per_grp);
            logbitmaps[g].bitmapSet(s & (nverts_per_grp-1));
        }

        std::string logfile = logname(base_filename, 0);
        appendfile(logfile, ebuffer, nebuf * 2 * sizeof(vid_t));
        nelogs += nebuf;
        nebuf = 0;
        
        size_t fsize = filesize(logfile);
        if(fsize >= logsize){ 
            // logstream(LOG_DEBUG) << "To compaction : nelogs = " << nelogs << std::endl;
            compaction();
            nelogs = 0;
            remove(logfile.c_str());
            for(bid_t g = 0; g < ngroups; g++){
                logbitmaps[g].bitmapReset();
            }
        }
        m.stop_time("_4_flush_3_writeLogs");
    }

    /***
    * Merge logs to a block subgraph(CSR)
    ***/
    void compaction(){
        m.start_time("_5_compaction_");
        
        m.start_time("_compaction_1_loadGraph");
        vid_t nverts = 0, *csr = NULL;
        eid_t nedges = 0, *beg_pos = NULL;
        loadGraph(beg_pos, csr, &nverts, &nedges);
        m.stop_time("_compaction_1_loadGraph");

        //8. Rewrite the CSR to disk
        m.start_time("_compaction_2_writeGraph");
        writeGraphCSR(csr, nedges, beg_pos, nverts);
        m.stop_time("_compaction_2_writeGraph");

        //9. Free memory
        m.start_time("_compaction_3_free");
        if(beg_pos != NULL) free(beg_pos);
        if(csr != NULL) free(csr);
        m.stop_time("_compaction_3_free");
        
        m.stop_time("_5_compaction_");

    }

    
    void loadGraph(eid_t * &beg_pos, vid_t * &csr, vid_t *nverts, eid_t *nedges){

        //1. Load the CSR from disk
        m.start_time("_load_1_loadCSR");
        *nverts = N;
        loadGraphCSR(beg_pos, csr, *nverts, nedges);
        m.stop_time("_load_1_loadCSR");

        //2. Load the logs of block_p
        m.start_time("_load_2_loadLog");
        vid_t* logs;
        eid_t nlogs = 0;
        std::string logfile = logname(base_filename, 0);
        nlogs = readfile(logfile, &logs) / (sizeof(vid_t)*2);
        m.stop_time("_load_2_loadLog");

        //2. compute new degree for each vertex
        m.start_time("_load_3_computeDegree");
        eid_t* newdeg = (eid_t*)malloc((*nverts)*sizeof(eid_t));
        memset(newdeg, 0, (*nverts)*sizeof(eid_t));
        for(eid_t e = 0; e < nlogs; e++){
            newdeg[logs[2*e]]++;
        }
        *nedges += nlogs;
        m.stop_time("_load_3_computeDegree");

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
        for(eid_t e = 0; e < nlogs; e++){
            vid_t v = logs[2*e];
            eid_t pos = newbeg_pos[v+1] - newdeg[v];
            newcsr[pos] = logs[2*e+1];
            newdeg[v]--;
        }
        m.stop_time("_load_6_mergeLog2CSR");

        
        m.start_time("_load_7_free");
        if(logs != NULL) free(logs);
        if(newdeg != NULL) free(newdeg);
        if(beg_pos != NULL) free(beg_pos);
        if(csr != NULL) free(csr);
        csr = (vid_t*)newcsr;
        beg_pos = (eid_t*)newbeg_pos;
        m.stop_time("_load_7_free");
    }

    void loadGraphCSR(eid_t * &beg_pos, vid_t * &csr, vid_t nverts, eid_t *nedges){
        std::string blkname = blockname(base_filename, 0);
        loadBegpos(blkname, beg_pos, nverts);
        *nedges = beg_pos[nverts] - beg_pos[0];
        // logstream(LOG_ERROR) << "loadSubGraphCSR : p = " << p << ": [" << blocks[p] << ", " << blocks[p+1] <<"), nverts = " << nverts << ", *nedges = " << *nedges << ", beg_pos[0] = " << beg_pos[0] << ", beg_pos[nverts] = " << beg_pos[nverts] << std::endl;
        loadCSR(blkname, csr, *nedges);
    }

    void loadBegpos(std::string bname, eid_t * &beg_pos, vid_t nverts, vid_t off = 0){

        beg_pos = (eid_t*)malloc((nverts+1)*sizeof(eid_t));

        std::string beg_posname = bname + ".beg_pos";
        FILE *tryf = fopen(beg_posname.c_str(), "r");
        if (tryf == NULL) { // Not found block beg_pos file
            // logstream(LOG_WARNING) << "Could not find the block beg_pos file : " << beg_posname << std::endl;
            memset(beg_pos, 0, (nverts+1)*sizeof(eid_t));
            return ;
        }
        fclose(tryf);

        int beg_posf = open(beg_posname.c_str(),O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
        if (beg_posf < 0) {
            logstream(LOG_FATAL) << "Could not load :" << beg_posname << ", error: " << strerror(errno) << std::endl;
        }
        assert(beg_posf > 0);

        /* read beg_pos file */
        size_t nread = preada(beg_posf, beg_pos, (size_t)(nverts+1)*sizeof(eid_t), (size_t)(off)*sizeof(eid_t));
        if(nread == 0) {
            memset(beg_pos, 0, (nverts+1)*sizeof(eid_t));
        }else{
            vid_t rnverts = nread / sizeof(eid_t) - 1;
            // logstream(LOG_WARNING) << "rnverts = " << rnverts << ", nverts = " << nverts << std::endl;
            for(vid_t v = rnverts+1; v <= nverts; v++)
            beg_pos[v] = beg_pos[rnverts];
        }

        close(beg_posf);
    }

    void loadCSR(std::string bname, vid_t * &csr, eid_t nedges, eid_t off = 0){
        if(nedges <= 0) return;

        std::string csrname = bname + ".csr";
        int csrf = open(csrname.c_str(), O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
        if (csrf < 0) {
            logstream(LOG_FATAL) << "Could not load :" << csrname << ", error: " << strerror(errno) << std::endl;
        }
        assert(csrf > 0);

        /* read csr file */
        csr = (vid_t*)malloc(nedges*sizeof(vid_t));
        size_t nread = preada(csrf, csr, nedges*sizeof(vid_t), off*sizeof(vid_t));
        assert(nread == nedges*sizeof(vid_t));

        close(csrf); 
    }

    void writeGraphCSR(vid_t* csr, eid_t nedges, eid_t* beg_pos, vid_t nverts){
        std::string blkname = blockname(base_filename, 0);
        std::string beg_posname = blkname + ".beg_pos";
        writefile(beg_posname, beg_pos, (size_t)(nverts+1)*sizeof(eid_t));
        std::string csrname = blkname + ".csr";
        writefile(csrname, csr, (size_t)nedges*sizeof(vid_t));
        // logstream(LOG_ERROR) << "writeSubGraphCSR : p = " << p << ": [" << blocks[p] << ", " << blocks[p+1] <<"), nverts = " << nverts << ", *nedges = " << nedges << ", beg_pos[0] = " << beg_pos[0] << ", beg_pos[nverts] = " << beg_pos[nverts] << std::endl;
    }

    std::vector<vid_t> getNeighbors(vid_t v){
        if(v >= N){
            logstream(LOG_ERROR) << "Invalid vertex id : " << v << " >= " << N << std::endl;
            std::vector<vid_t> neighbors;
            return neighbors;
        }

        m.start_time("test_searchNeighbors_1_InCSR");
        std::vector<vid_t> neighbors = getNeighborsInCSR(v);
        // logstream(LOG_DEBUG) << "After loadBlock, # of neighbors = " << neighbors.size() << std::endl;
        m.stop_time("test_searchNeighbors_1_InCSR");

        // load and search logs
        bid_t g = (bid_t)(v >> nbits_nverts_per_grp);
        if(logbitmaps[g].bitmapGet( v & (nverts_per_grp-1) )){
            m.start_time("test_searchNeighbors_2_InLogfile");
            m.start_time("test_searchNeighbors_2_InLogfile_1_readfile");
            // std::string logfile = segmentname(base_filename, p, segs[p][s]) + ".log";
            std::string logfile = logname(base_filename, 0);
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
            m.stop_time("test_searchNeighbors_2_InLogfile_3_freeLog");
            m.stop_time("test_searchNeighbors_2_InLogfile");
        }
        // logstream(LOG_DEBUG) << "After loadLog, # of neighbors = " << neighbors.size() << std::endl;

        // search logs of memory edge buffer
        m.start_time("test_searchNeighbors_3_InMembuf");
        if(membitmaps[g].bitmapGet( v & (nverts_per_grp-1) )){
            for(eid_t e = 0; e < bufcap; e++){
                if(ebuffer[2*e] == v){
                    neighbors.push_back(ebuffer[2*e+1]);
                }
            }
        }
        // logstream(LOG_DEBUG) << "After searchBuffer, # of neighbors = " << neighbors.size() << std::endl;
        m.stop_time("test_searchNeighbors_3_InMembuf");

        return neighbors;
    }

    std::vector<vid_t> getNeighborsInCSR(vid_t v){
        std::vector<vid_t> neighbors;

        std::string blkname = blockname(base_filename, 0);
        eid_t *beg_pos;// = (eid_t*)malloc(2*sizeof(eid_t));
        vid_t off = v;
        loadBegpos(blkname, beg_pos, 1, off);

        eid_t nedges = beg_pos[1] - beg_pos[0];
        if(nedges > 0){
            vid_t *csr;// = (vid_t*)malloc(nedges*sizeof(vid_t));
            loadCSR(blkname, csr, nedges, beg_pos[0]);
            for(eid_t i = 0; i < nedges; i++){
                neighbors.push_back(csr[i]);
            }
            if(csr != NULL) free(csr);
        }
        if(beg_pos != NULL) free(beg_pos);

        return neighbors;
    }


};

#endif