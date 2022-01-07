#ifndef DATATYPE_DEF
#define DATATYPE_DEF

#include <vector>
#include <stdint.h>
#include "logger/logger.hpp"

#define	RAND_MAX	2147483647
#define	FILE_SIZE	1024 // GB
#define	VERT_SIZE	64 * 1024 * 1024 // 64M vertices in beg_pos buffer in preprocess
#define	MAX_SOURCE_SIZE	16 * 1024 * 1024 // 
#define	MAX_WALK_NUM	0xffffffffffffffff // 
#define	EDGE_SIZE	256 * 1024 * 1024 // 256M edges in csr buffer in preprocess
#define	WALK_BUFFER_SIZE	4 * 1024 // most 1024 walks in a in-memory walk buffer
#define	MEM_BUDGET	44 * 1024 * 1024 // for 64GB memory machine
// #define	MEM_BUDGET	4 * 1024 * 1024 // for 8GB memory machine

typedef uint32_t vid_t;
typedef uint64_t eid_t;
typedef uint64_t wid_t; //type of id of walks
typedef uint32_t bid_t; //type of id of blocks
typedef uint16_t hid_t; //type of id of hops
typedef uint8_t tid_t; //type of id of threads
typedef unsigned VertexDataType;

class mem_pool{
public:
    char* head;
    size_t size;
    mem_pool(size_t size1){
        size = size1;
        head = (char*)malloc(size);
    }
    char* pool_alloc(size_t size1){
        assert(size1 <= size);
        char* addr = head;
        head += size1;
        size -= size1;
        return addr;
    }
    void reset(char* head1, size_t size1){
        head = head1;
        size = size1;
    }
};

struct block_index{
    vid_t nverts;
    eid_t nedges;
    eid_t* begpos;
    vid_t* csr;
    size_t csr_size;
    bid_t stb, enb;
    block_index(){ nverts = 0; nedges = 0; begpos = NULL; csr = NULL; csr_size = 0, stb = enb = 0;}
};

size_t alignment(size_t size, size_t align){
    if(size % align == 0) return size;
    return (size / align + 1) * align;
}

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

#endif