#ifndef DATATYPE_DEF
#define DATATYPE_DEF

#include <vector>
#include <stdint.h>

typedef uint32_t vid_t;
typedef float VertexDataType;

struct Vertex{
    vid_t vid;
    int outd;
    std::vector<vid_t> outv;	
};

 vid_t random_outneighbor( Vertex v) {
    return v.outv[((int) std::abs(random()) % v.outd)];
    //return v.outv[(int) (std::abs(random()) % v.outd)];
}
#endif