#ifndef DATATYPE_DEF
#define DATATYPE_DEF

#include <vector>
#include <stdint.h>
#include "logger/logger.hpp"

#define	RAND_MAX 2147483647

typedef uint16_t sid_t; // shard id
typedef uint32_t vid_t; // vertex id
typedef uint64_t eid_t; // edge id
typedef uint32_t wid_t; // walk id
typedef uint16_t hid_t; // hop id
typedef unsigned VertexDataType;
typedef unsigned long WalkDataType;

struct Vertex{
    vid_t vid;
    int outd;
    vid_t *outv;	
    // std::vector<vid_t> outv;	
    // ~Vertex(){
    //     free(outv);
    // }
};

#endif