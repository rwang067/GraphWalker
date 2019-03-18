#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <sys/time.h> 
#include <stdio.h>
#include <math.h> 

#include "walks/randomwalk.hpp"

#define maxwalklength 6

__device__ vid_t getSourceId( WalkDataType walk ){
    return (vid_t)( walk >> 40 ) & 0xffffff;
}

__device__ vid_t getCurrentId( WalkDataType walk ){
    return (vid_t)( walk >> 14 ) & 0x3ffffff;
}

__device__ unsigned getHop( WalkDataType walk ){
    return (unsigned)(walk & 0x3fff) ;
}

__device__ WalkDataType encode( vid_t sourceId, vid_t currentId, unsigned hop ){
    assert( hop < 16384 );
    return (( (WalkDataType)sourceId & 0xffffff ) << 40 ) |(( (WalkDataType)currentId & 0x3ffffff ) << 14 ) | ( (WalkDataType)hop & 0x3fff ) ;
}

__device__ int gpu_rand_r(unsigned int *seed){
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
 
__global__ void updatebywalk(unsigned exec_interval, unsigned* intervals, unsigned* beg_pos, unsigned* csr, WalkDataType* walks, WalkDataType** pwalks, unsigned nvertices, unsigned nwalks, unsigned nshards){
    int i = blockDim.x * blockIdx.x + threadIdx.x;
    while( i < nwalks ){
    printf("\nupdatebywalk : %d \n", i );
        WalkDataType nowWalk = walks[i];
        //random walk
        vid_t sourId = getSourceId(nowWalk);
        vid_t dstId = getCurrentId(nowWalk) + intervals[exec_interval];
        unsigned hop = getHop(nowWalk);
        unsigned seed = i+dstId+hop; //+cur_time;
        printf("%d ， [%d, %d], %d \n", dstId, intervals[exec_interval], intervals[exec_interval+1], hop );
        while (dstId >= intervals[exec_interval] && dstId < intervals[exec_interval+1] && hop < maxwalklength ){
            // std::cout  << " -> " << dstId << " " << getSourceId(nowWalk) << std::endl;
            // updateInfo(sourId, dstId, threadid, hop);
            unsigned outd = beg_pos[dstId+1] - beg_pos[dstId];
            printf("%d : outd : %d \n ", i, outd );
            if (outd > 0 ){//&& ((float)gpu_rand_r(&seed))/RAND_MAX > 0.15 ){
                printf("i = %d, dstId = %d , beg_pos[dstId] = %d-- ", i, dstId, beg_pos[dstId] );
                unsigned pos = beg_pos[dstId] + ((unsigned)gpu_rand_r(&seed))%outd;
                dstId = csr[pos];
                printf(" pos = %d move to --> %d\n", pos, dstId );
            }else{
                printf("%d : Reset!\n", i);
                break;
            }
            hop++;
            nowWalk++;
        }
        if( hop < maxwalklength ){
            unsigned p = 0;
            for(; p < nshards; p++){
                if(dstId < intervals[p]) break;
            }
            nowWalk = encode(sourId, dstId-intervals[p], hop);
            // pwalks[p].push_back(nowWalk);
            // walk_manager.setMinStep( p, hop );
        }

        //Next walk
        i += 28*1024;
    }
    
    
}
 
//int exec_update(RandomWalk &userprogram, Vertex *&vertices, WalkManager &walk_manager )
int main(){
    struct timeval start, end;
    gettimeofday( &start, NULL );

    //input data
    unsigned nvertices = 3; // number of vertices in current interval
    unsigned nedges = 7; // number of vertices in current interval
    unsigned nwalks = 5; // number of walks in current interval
    unsigned nshards = 3; // number of shards

    unsigned intervals[] = {0, 3, 6, 10};
    unsigned beg_pos[] = {0, 3, 4, 7}; //interval 0
    unsigned csr[] = {1,3,4,2,0,6,7};
    WalkDataType walks[] = {0,0,2,16384,16385}; // walks in current interval
    WalkDataType** pwalks; // walks in current interval

    //define the variables used in GPU
    unsigned* d_intervals;
    unsigned* d_beg_pos; //interval 0
    unsigned* d_csr;
    WalkDataType* d_walks; // walks in current interval copied to GPU
    WalkDataType** d_pwalks; //walks moved to other intervals

    std::cout << "before malloc device memory" << std::endl;
    //malloc device memory
    cudaMalloc((void**)&d_intervals, sizeof(unsigned) * (nshards+1));
    cudaMalloc((void**)&d_beg_pos, sizeof(unsigned) * (nvertices+1));
    cudaMalloc((void**)&d_csr, sizeof(unsigned) * nedges);
    cudaMalloc((void**)&d_walks, sizeof(WalkDataType) * nwalks);

    std::cout << "before malloc pwalks memory" << std::endl;
    pwalks = (WalkDataType**)malloc(sizeof(WalkDataType*) * nshards);
    // cudaMalloc((void**)&d_pwalks, sizeof(WalkDataType*) * nshards);
    for(unsigned p = 0; p < nshards; p++){
        pwalks[p] = (WalkDataType*)malloc(sizeof(WalkDataType) * nwalks);
        // cudaMalloc((void**)&d_pwalks[p], sizeof(WalkDataType) * nwalks);
    }
    size_t pitch;
    cudaMallocPitch((void**)&d_pwalks, &pitch, sizeof(WalkDataType) * nwalks, nshards);

    std::cout << "before cudaMemcpy" << std::endl;
    cudaMemcpy(d_intervals, intervals, sizeof(unsigned)*(nshards+1), cudaMemcpyHostToDevice);
    cudaMemcpy(d_beg_pos, beg_pos, sizeof(unsigned)*(nshards+1), cudaMemcpyHostToDevice);
    cudaMemcpy(d_csr, csr, sizeof(unsigned)*nedges, cudaMemcpyHostToDevice);
    cudaMemcpy(d_walks, walks, sizeof(WalkDataType)*nwalks, cudaMemcpyHostToDevice);

    // 定义kernel执行配置，28个block，每个block里面有1024个线程
    dim3 dimGrid(28);
    dim3 dimBlock(1024);

    std::cout << "before updatebywalk" << std::endl;
    //conduct random walk moving
    updatebywalk <<<dimGrid, dimBlock>>> (0, d_intervals, d_beg_pos, d_csr, d_walks, d_pwalks, nvertices, nwalks, nshards);

    //拷贝计算数据-一级数据指针
    cudaMemcpy2D(pwalks, sizeof(WalkDataType) * nwalks, d_pwalks, pitch, sizeof(WalkDataType) * nwalks, nshards, cudaMemcpyDeviceToHost);
    // cudaMemcpy(pwalks, d_pwalks, sizeof(WalkDataType) * nwalks, cudaMemcpyDeviceToHost);

    //释放主机内存
    // free(intervals);
    // free(beg_pos);
    // free(csr);
    // free(walks);
    std::cout << "\nbefore free(pwalks)[0]" << std::endl;
    for(unsigned p = 0; p < nshards; p++)
        free(pwalks[p]);
    // std::cout << "before free(pwalks)" << std::endl;
    // free(pwalks);
    //释放设备内存
    std::cout << "before cudaFree(d_intervals);" << std::endl;
    cudaFree(d_intervals);
    cudaFree(d_beg_pos);
    cudaFree(d_csr);
    cudaFree(d_walks);
    std::cout << "before cudaFree(d_pwalks);" << std::endl;
    cudaFree(d_pwalks);
    // for(unsigned p = 0; p < nshards; p++)
    //     cudaFree(d_pwalks[p]);

    gettimeofday( &end, NULL );
    int timeuse = 1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
    printf("total time in exec_update is %d ms\n", timeuse/1000);

    return 0;
}
