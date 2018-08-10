#ifndef DATATYPE_DEF
#define DATATYPE_DEF

#include <vector>
#include <stdint.h>
#include "logger/logger.hpp"

typedef uint32_t vid_t;
typedef float VertexDataType;

struct Vertex{
    vid_t vid;
    int outd;
    vid_t *outv;	
    // std::vector<vid_t> outv;	
    // ~Vertex(){
    //     free(outv);
    // }
};

 vid_t random_outneighbor( Vertex v, unsigned& seed) {
    if(false){
        logstream(LOG_INFO) << "Vertex = " << v.vid << " , d = " << v.outd << " , out_neighbors = ";
        for( int i = 0; i < v.outd; i++ )
            logstream(LOG_INFO) << v.outv[i] << " , ";
        logstream(LOG_INFO) << std::endl;
    }
    int ran = rand_r(&seed) % v.outd;
    // int ran = rand() % v.outd; //rand() cause data race in muti-threads 
    return v.outv[ran];
}
#endif