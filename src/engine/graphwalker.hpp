
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
        walk_manager = new WalkManager(m);
        walk_manager->initialnizeWalks(nshards, base_filename);

        _m.set("file", _base_filename);
        _m.set("engine", "default");
        _m.set("nshards", (size_t)nshards);
    }
        
    virtual ~graphwalker_engine() {
    }

    void loadSubGraph(int p, std::vector<Vertex> &vertices ){
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
        int curvertex = intervals[p].first;
        vertices.clear();
        while( vcnt > 0 ){
            Vertex v;
            v.vid = curvertex;
            int dcnt = *((int*)bufptr);
            bufptr += sizeof(int);
            v.outd = dcnt;
            for( int i = 0; i < v.outd; i++ ){
                vid_t to = *((vid_t*)bufptr);
                bufptr += sizeof(vid_t);
                v.outv.push_back(to);
            }
            vertices.push_back(v);
            curvertex++;
            vcnt -= (v.outd + 1);
        }
        free(buf);
        close(inf);
    }

    void initialnizeVertexData(){
        std::cout << "initialnizeVertexData " << std::endl;
        char *vertex_value = (char *)malloc(sizeof(VertexDataType)*nvertices);
        char *vertex_valueptr = vertex_value;
        for( int i = 0; i < nvertices; i++ ){
            // vertex_value[i] = i;
            *((VertexDataType*)vertex_valueptr) = 0;
            // std::cout << i << "  " << vertex_value[i] << std::endl;
            // std::cout << i << "  " << *vertex_valueptr << std::endl;
             vertex_valueptr += sizeof(VertexDataType);
        }
        std::string vertex_value_file = filename_vertex_data(base_filename);
        std::cout << vertex_value_file << std::endl;
        writefile(vertex_value_file, vertex_value, vertex_valueptr);
        std::cout << "initialnizeVertexData  end" << std::endl;
        free(vertex_value);
    }

    static VARIABLE_IS_NOT_USED void load_vertex_intervals(std::string base_filename, int nshards, std::vector<std::pair<vid_t, vid_t> > & intervals, bool allowfail=false) {
        std::string intervalsFilename = filename_intervals(base_filename, nshards);
        std::cout << intervalsFilename << std::endl;
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

    void exec_updates(RandomWalk &userprogram, std::vector<Vertex> vertices ){ //, VertexDataType* vertex_value){
        int count = walk_manager->readIntervalWalks(exec_interval);
        m.start_time("exec_updates");
        // exec_threads = 1;
        omp_set_num_threads(exec_threads);
        #pragma omp parallel for schedule(dynamic)
            for( int i = 0; i < count; i++ ){
                // std::cout << "walk : " << i << " --> threads." << omp_get_thread_num() << std::endl;
                WalkDataType walk = walk_manager->curwalks[i];
                userprogram.updateByWalk(walk, exec_interval, vertices, *walk_manager );//, vertex_value);
            }
        // #pragma omp barrier
        m.stop_time("exec_updates");
        walk_manager->writeIntervalWalks(exec_interval);
    }

    void run(RandomWalk &userprogram, float prob) {
        srand((unsigned)time(NULL));
        m.start_time("runtime");
        userprogram.startWalks(*walk_manager, nvertices, intervals);
        //initialnizeVertexData();

        /*loadOnDemand -- Interval loop */
        int numIntervals = 0;
        std::vector<Vertex> vertices;
        while( userprogram.hasFinishedWalk(*walk_manager) ){
            numIntervals++;
            float cc = ((float)rand())/RAND_MAX;
            std::cout << cc << std::endl;
            if( cc < prob ){
                exec_interval = walk_manager->intervalWithMinStep();
            }else{
                exec_interval =walk_manager->intervalWithMaxWalks();
            }
            logstream(LOG_INFO) << "numIntervals: " << numIntervals << " : " << exec_interval << std::endl;
            //walk_manager->printWalksDistribution( exec_interval );
            /*runInterval*/
            metrics_entry mr = m.start_time();
            vertices.clear();
            /*load graph info*/
            loadSubGraph(exec_interval, vertices);

            // exec_updates( userprogram, vertices, vertex_value );
            userprogram.before_exec_interval(intervals[exec_interval].first, intervals[exec_interval].second);
            exec_updates(userprogram, vertices);
            userprogram.after_exec_interval(intervals[exec_interval].first, intervals[exec_interval].second);

            m.stop_time(mr, "_in_run_interval");
        } // For Interval loop
        m.stop_time("runtime");
    }
};

#endif