
#ifndef DEF_GRAPHCHWALKER_ENGINE
#define DEF_GRAPHCHWALKER_ENGINE

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <omp.h>
#include <vector>
#include <map>
#include <sys/time.h>

#include "api/filename.hpp"
#include "api/io.hpp"
#include "logger/logger.hpp"
#include "metrics/metrics.hpp"
#include "api/pthread_tools.hpp"
#include "walks/randomwalk.hpp"

class graphwalker_engine {
public:     
    std::string base_filename;
    int nshards;  
    int nvertices;      
    size_t blocksize;
    int membudget_mb;
    int exec_threads;
    std::vector<std::pair<vid_t, vid_t> > intervals;
    
    /* State */
    int exec_interval;
    
    /* Metrics */
    metrics &m;
    WalkManager *walk_manager;
        
    void print_config() {
        logstream(LOG_INFO) << "Engine configuration: " << std::endl;
        logstream(LOG_INFO) << " exec_threads = " << exec_threads << std::endl;
        // logstream(LOG_INFO) << " load_threads = " << load_threads << std::endl;
        logstream(LOG_INFO) << " membudget_mb = " << membudget_mb << std::endl;
        logstream(LOG_INFO) << " blocksize = " << blocksize << std::endl;
        // logstream(LOG_INFO) << " scheduler = " << use_selective_scheduling << std::endl;
    }
        
public:
        
    /**
     * Initialize GraphChi engine
     * @param base_filename prefix of the graph files
     * @param nshards number of shards
     * @param selective_scheduling if true, uses selective scheduling 
     */
    graphwalker_engine(std::string _base_filename, int _nshards, metrics &_m) : base_filename(_base_filename), nshards(_nshards), m(_m) {
        // m.start_time("iomgr_init");
        // iomgr = new stripedio(m);
        // m.stop_time("iomgr_init");

        membudget_mb = get_option_int("membudget_mb", 1024);
        exec_threads = get_option_int("execthreads", omp_get_max_threads());

        load_vertex_intervals(base_filename, nshards, intervals);
        nvertices = num_vertices();
        walk_manager = new WalkManager(m,nshards,exec_threads,base_filename);

        _m.set("file", _base_filename);
        _m.set("engine", "default");
        _m.set("nshards", (size_t)nshards);
    }
        
    virtual ~graphwalker_engine() {
    }

    void loadSubGraph(int p, Vertex *&vertices ){
        m.start_time("loadSubGraph");
        std::string invlname = intervalname( base_filename, p );
        int inf = open(invlname.c_str(),O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
        if (inf < 0) {
            logstream(LOG_FATAL) << "Could not load :" << invlname << " error: " << strerror(errno) << std::endl;
        }
        assert(inf > 0);
        char * buf;// = (char*) malloc(invlsize*sizeof(int));
        size_t sz = readfull(inf, &buf);
        char * bufptr = buf;
        int vcnt = sz / sizeof(int);
        int curvertex = 0;//intervals[p].first;
        // vertices.clear();
        int numv = (intervals[p].second-intervals[p].first+1);
        // logstream(LOG_INFO) << "numv = " << numv << std::endl;
        vertices = (Vertex*)malloc((numv+1)*sizeof(Vertex));
        while( vcnt > 0 ){
            Vertex v;
            v.vid = curvertex + intervals[p].first;
            int dcnt = *((int*)bufptr);
            bufptr += sizeof(int);
            v.outd = 0;
            v.outv = (vid_t*)malloc(dcnt*sizeof(vid_t));
            for( int i = 0; i < dcnt; i++ ){
                vid_t to = *((vid_t*)bufptr);
                bufptr += sizeof(vid_t);
                // logstream(LOG_WARNING) << "v.outd, dcnt :" << v.outd << " , " << dcnt << std::endl;
                v.outv[v.outd++] = to;
                // v.outv.push_back(to);
            }
            // if(curvertex < 3 || curvertex >= numv-3 ){
            //     logstream(LOG_INFO) << "Vertex = " << v.vid << " , d = " << v.outd << " , out_neighbors = ";
            //     for( int i = 0; i < v.outd; i++ )
            //         logstream(LOG_INFO) << v.outv[i] << " , ";
            //     logstream(LOG_INFO) << std::endl;
            // }
            // vertices.push_back(v);
            // logstream(LOG_WARNING) << "curvertex :" << curvertex << std::endl;
            vertices[curvertex++] = v;
            // curvertex++;
            vcnt -= (v.outd + 1);
            // free(v.outv);
        }
        free(buf);
        close(inf);
        m.stop_time("loadSubGraph");
    }

    void initialnizeVertexData(){
        logstream(LOG_INFO) << "initialnizeVertexData " << std::endl;
        char *vertex_value = (char *)malloc(sizeof(VertexDataType)*nvertices);
        char *vertex_valueptr = vertex_value;
        for( int i = 0; i < nvertices; i++ ){
            // vertex_value[i] = i;
            *((VertexDataType*)vertex_valueptr) = 0;
            // logstream(LOG_INFO) << i << "  " << vertex_value[i] << std::endl;
            // logstream(LOG_INFO) << i << "  " << *vertex_valueptr << std::endl;
             vertex_valueptr += sizeof(VertexDataType);
        }
        std::string vertex_value_file = filename_vertex_data(base_filename);
        logstream(LOG_INFO) << vertex_value_file << std::endl;
        writefile(vertex_value_file, vertex_value, vertex_valueptr);
        logstream(LOG_INFO) << "initialnizeVertexData  end" << std::endl;
        free(vertex_value);
    }

    static VARIABLE_IS_NOT_USED void load_vertex_intervals(std::string base_filename, int nshards, std::vector<std::pair<vid_t, vid_t> > & intervals, bool allowfail=false) {
        std::string intervalsFilename = filename_intervals(base_filename, nshards);
        // logstream(LOG_INFO) << intervalsFilename << std::endl;
        std::ifstream intervalsF(intervalsFilename.c_str());
        
        if (!intervalsF.good()) {
            logstream(LOG_ERROR) << "Could not load intervals-file: " << intervalsFilename << std::endl;
        }
        assert(intervalsF.good());
        
        intervals.clear();
        
        vid_t st=0, en;
        for(int i=0; i < nshards; i++) {
            assert(!intervalsF.eof());
            intervalsF >> en;
            intervals.push_back(std::pair<vid_t,vid_t>(st, en));
            st = en + 1;
        }
        for(int i=0; i < nshards; i++) {
             logstream(LOG_INFO) << "shard: " << intervals[i].first << " - " << intervals[i].second << std::endl;
        }
        intervalsF.close();
    }

    virtual size_t num_vertices() {
        return 1 + intervals[nshards - 1].second;
    }

    void exec_updates(RandomWalk &userprogram, Vertex *&vertices ){ //, VertexDataType* vertex_value){
        int count = walk_manager->readIntervalWalks(exec_interval);
        logstream(LOG_INFO) << "readIntervalWalks of exec_interval : " << exec_interval << " : " << count << std::endl;
        m.start_time("exec_updates");
        // exec_threads = 1;
        omp_set_num_threads(exec_threads);
        // #pragma omp parallel for schedule(dynamic)
        #pragma omp parallel for schedule(static)
            for( int i = 0; i < count; i++ ){
                // logstream(LOG_INFO) << "exec_interval : " << exec_interval << " , walk : " << i << " --> threads." << omp_get_thread_num() << std::endl;
                WalkDataType walk = walk_manager->pwalks[0][exec_interval][i];
                userprogram.updateByWalk(walk, exec_interval, vertices, *walk_manager );//, vertex_value);
            }
        // #pragma omp barrier
        m.stop_time("exec_updates");
        walk_manager->writeIntervalWalks(exec_interval);
    }

    void run(RandomWalk &userprogram, float prob) {
        m.start_time("runtime");
        // srand((unsigned)time(NULL));
        m.start_time("startWalks");
        userprogram.startWalks(*walk_manager, nvertices, intervals);
        m.stop_time("startWalks");
        //initialnizeVertexData();

        /*loadOnDemand -- Interval loop */
        int numIntervals = 0;
        Vertex *vertices;
        while( userprogram.hasFinishedWalk(*walk_manager) ){
            m.start_time("in_run_interval");
            numIntervals++;
            float cc = ((float)rand())/RAND_MAX;
            // logstream(LOG_DEBUG) << "proc < 0.2 --> minstep, choose probability = " << cc << std::endl;
            if( cc < prob ){
                exec_interval = walk_manager->intervalWithMinStep();
            }else{
                exec_interval =walk_manager->intervalWithMaxWalks();
            }
            logstream(LOG_DEBUG) << "numIntervals: " << numIntervals << " : " << exec_interval << std::endl;
            //walk_manager->printWalksDistribution( exec_interval );
            /*load graph info*/
            loadSubGraph(exec_interval, vertices);
            logstream(LOG_INFO) << "loadSubGraph finish " << std::endl;
            userprogram.before_exec_interval(intervals[exec_interval].first, intervals[exec_interval].second);
            exec_updates(userprogram, vertices);
            logstream(LOG_INFO) << "exec_updates finish " << std::endl;
            userprogram.after_exec_interval(intervals[exec_interval].first, intervals[exec_interval].second);

            m.start_time("free vertices");
            for(unsigned i = 0; i < (intervals[exec_interval].second-intervals[exec_interval].first+1); i++)
                free(vertices[i].outv);
            free(vertices);
            m.stop_time("free vertices");

            m.stop_time("in_run_interval");
        } // For Interval loop
        m.stop_time("runtime");
    }
};

#endif