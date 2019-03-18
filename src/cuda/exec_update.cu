#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <sys/time.h> 
#include <stdio.h>
#include <math.h> 

#include "api/datatype.hpp"

#define maxwalklength 10

__device__ vid_t getSourceId( WalkDataType walk ){
    return (vid_t)( walk >> 40 ) & 0xffffff;
}

__device__ vid_t getCurrentId( WalkDataType walk ){
    return (vid_t)( walk >> 14 ) & 0x3ffffff;
}

__device__ hid_t getHop( WalkDataType walk ){
    return (hid_t)(walk & 0x3fff) ;
}

__device__ WalkDataType encode( vid_t sourceId, vid_t currentId, hid_t hop ){
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
 
__global__ void updatebywalk(sid_t exec_interval, vid_t* intervals, eid_t* beg_pos, vid_t* csr, WalkDataType* walks, WalkDataType* pwalks, wid_t* pnwalks, vid_t nverts, wid_t nwalks, sid_t nshards){
    unsigned i = blockDim.x * blockIdx.x + threadIdx.x;
    while( i < nwalks ){
        // if(blockDim.x * blockIdx.x + threadIdx.x == 0) {
            // printf("\nin GPU, pnwalks[0] = %d, ", pnwalks[0] );
            // printf("pnwalks[1] = %d\n ", pnwalks[1] );
            // printf("pwalks[0] = %d\n ", pwalks[0] );
            // printf("nwalks = %d\n ", nwalks );
            WalkDataType nowWalk = walks[i];
            //random walk
            vid_t sourId = getSourceId(nowWalk);
            vid_t dstId = getCurrentId(nowWalk) + intervals[exec_interval];
            hid_t hop = getHop(nowWalk);
            unsigned seed = i+dstId+hop; //+cur_time;
            bool reset = false;
            // printf("%d ， [%d, %d), %d \n", dstId, intervals[exec_interval], intervals[exec_interval+1], hop );
            while (dstId >= intervals[exec_interval] && dstId < intervals[exec_interval+1] && hop < maxwalklength ){
                // std::cout  << " -> " << dstId << " " << getSourceId(nowWalk) << std::endl;
                // updateInfo(sourId, dstId, threadid, hop);
                eid_t outd = beg_pos[dstId+1] - beg_pos[dstId];
                // printf("dstId = %d\n ", dstId );
                // printf("outd = %d\n ", outd );
                if (outd > 0 && ((float)gpu_rand_r(&seed))/RAND_MAX > 0.15 ){
                    eid_t pos = beg_pos[dstId] + ((eid_t)gpu_rand_r(&seed))%outd;
                    dstId = csr[pos];
                    // printf(" pos = %d", pos );
                    // printf(" move to --> %d\n", dstId );
                }else{
                    // printf("%d : Reset!\n", i);
                    reset = true;
                    break;
                }
                hop++;
            }
            // printf("hop = %d, ", hop );
            // printf("maxwalklength = %d\n ", maxwalklength );
            if( hop < maxwalklength && !reset ){
                sid_t p;
                for(p = 0; p < nshards; p++){
                    if(dstId < intervals[p+1]) {
                        // printf("p = %d, ", p );
                        // printf("dstId = %d\n ", dstId );
                        break;
                    }
                }
                // printf("after break : dstId = %d\n ", dstId );
                nowWalk = encode(sourId, dstId-intervals[p], hop);
                // printf("after encode : nowWalk = %d\n ", nowWalk );
                // printf("before : p = %d, ", p );
                // printf("pnwalks[p] = %d\n ", pnwalks[p] );
                wid_t w = p*nwalks + pnwalks[p]++;
                pwalks[w] = nowWalk;
                // printf("after : w = %d, ", w );
                // printf("pnwalks[p] = %d\n ", pnwalks[p] );
                // walk_manager.setMinStep( p, hop );
            }
        // }
        //Next walk
        i += 28*1024;
        // printf("i = %d, ", i );
        // printf("nwalks = %d\n ", nwalks );
    }
}
 
//int exec_update(RandomWalk &userprogram, Vertex *&vertices, WalkManager &walk_manager )
// int main(){
void exec_updates(eid_t *beg_pos, vid_t *csr, sid_t exec_interval, vid_t* intervals, WalkDataType* walks, WalkDataType **&pwalks, wid_t *&pnwalks, vid_t nverts, eid_t nedges, wid_t nwalks, sid_t nshards){
    struct timeval start, end;
    gettimeofday( &start, NULL );

    // printf("in exec_updates : exec_interval = %d, nshards = %d, nverts = %d, , nedges = %d, nwalks = %d\n", exec_interval, nshards, nverts, nedges, nwalks);

    //define the variables used in GPU
    vid_t* d_intervals;
    eid_t* d_beg_pos;
    vid_t* d_csr;
    WalkDataType* d_walks; // walks in current interval copied to GPU
    WalkDataType* d_pwalks; //walks moved to other intervals
    wid_t* d_pnwalks;

    // std::cout << "before malloc device memory" << std::endl;
    //malloc device memory
    cudaMalloc((void**)&d_intervals, sizeof(vid_t) * (nshards+1));
    cudaMalloc((void**)&d_beg_pos, sizeof(eid_t) * (nverts+1));
    cudaMalloc((void**)&d_csr, sizeof(vid_t) * nedges);
    cudaMalloc((void**)&d_walks, sizeof(WalkDataType) * nwalks);

    cudaMalloc((void**)&d_pnwalks, sizeof(wid_t) * nshards);
    cudaMemset(d_pnwalks, 0, sizeof(wid_t) * nshards);

    cudaMalloc((void**)&d_pwalks, sizeof(WalkDataType) * nshards * nwalks);
    cudaMemset(d_pwalks, 0, sizeof(WalkDataType) * nshards * nwalks);

    // std::cout << "before cudaMemcpy" << std::endl;
    cudaMemcpy(d_intervals, intervals, sizeof(vid_t)*(nshards+1), cudaMemcpyHostToDevice);
    cudaMemcpy(d_beg_pos, beg_pos, sizeof(eid_t)*(nverts+1), cudaMemcpyHostToDevice);
    cudaMemcpy(d_csr, csr, sizeof(vid_t)*nedges, cudaMemcpyHostToDevice);
    cudaMemcpy(d_walks, walks, sizeof(WalkDataType)*nwalks, cudaMemcpyHostToDevice);
    cudaMemcpy(d_pnwalks, pnwalks, sizeof(wid_t)*nshards, cudaMemcpyHostToDevice);

    // printf("in CPU, pnwalks[0] = %d, ", pnwalks[0] );
    // printf("pnwalks[1] = %d\n ", pnwalks[1] );

    // 定义kernel执行配置，28个block，每个block里面有1024个线程
    dim3 dimGrid(28);
    dim3 dimBlock(1024);

	// cudaMemcpy2D(d_pwalks, pitch, pwalks, sizeof(WalkDataType) * nwalks, sizeof(WalkDataType) * nwalks, nshards, cudaMemcpyHostToDevice);
    
    //conduct random walk moving
    // std::cout << "before updatebywalk" << std::endl;
    updatebywalk<<<dimGrid, dimBlock>>>(exec_interval, d_intervals, d_beg_pos, d_csr, d_walks, d_pwalks, d_pnwalks, nverts, nwalks, nshards);

	// 将device端数据拷贝到host端返回数据
    std::cout << "before cudaMemcpyDeviceToHost;" << std::endl;
    cudaMemcpy(pnwalks, d_pnwalks, sizeof(wid_t) * nshards, cudaMemcpyDeviceToHost);
    for(sid_t p = 0; p < nshards; p++){
        printf("pnwalks[%d] = %d, \n", p, pnwalks[p]);
        pwalks[p] = (WalkDataType*)malloc(sizeof(WalkDataType) * pnwalks[p]);
        cudaMemcpy(pwalks[p], d_pwalks, sizeof(WalkDataType) * pnwalks[p], cudaMemcpyDeviceToHost);
    }

    //释放设备内存
    std::cout << "before cudaFree(d_intervals);" << std::endl;
    cudaFree(d_intervals);
    cudaFree(d_beg_pos);
    cudaFree(d_csr);
    cudaFree(d_walks);
    cudaFree(d_pnwalks);
    cudaFree(d_pwalks);

    gettimeofday( &end, NULL );
    // int timeuse = 1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
    // printf("total time in exec_update is %d ms\n", timeuse/1000);

}
