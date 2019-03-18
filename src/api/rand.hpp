#ifndef RAND_DEF
#define RAND_DEF

#include <vector>
#include "logger/logger.hpp"

int my_rand_r (unsigned int *seed){
    unsigned int next = *seed;
    int result;

    next *= 1103515245;
    next += 12345;
    result = (unsigned int) (next / 65536) % 2048;

    next *= 1103515245;
    next += 12345;
    result <<= 10;
    result ^= (unsigned int) (next / 65536) % 1024;

    next *= 1103515245;
    next += 12345;
    result <<= 10;
    result ^= (unsigned int) (next / 65536) % 1024;

    *seed = next;

    return result;
}

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