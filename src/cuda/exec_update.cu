#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <sys/time.h> 
#include <stdio.h>
#include <math.h> 

#include "api/datatype.hpp"
#include "metrics/metrics.hpp"

#define maxwalklength 10
#define walkpitch 1000
#define nthreads 28*1024

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
 
__global__ void updatebywalk(sid_t exec_interval, vid_t* intervals, eid_t* beg_pos, vid_t* csr, WalkDataType* walks, WalkDataType* tpwalks, wid_t* tpnwalks, vid_t nverts, wid_t nwalks, sid_t nshards){
    sid_t tid = blockDim.x * blockIdx.x + threadIdx.x;
    unsigned i = tid;
    while( i < nwalks ){
        WalkDataType nowWalk = walks[i];
        vid_t sourId = getSourceId(nowWalk);
        vid_t dstId = getCurrentId(nowWalk) + intervals[exec_interval];
        hid_t hop = getHop(nowWalk);
        unsigned seed = i+dstId+hop; //+cur_time;
        bool reset = false;
        if (dstId < intervals[exec_interval] || dstId >= intervals[exec_interval+1] || hop >= maxwalklength ){
        }
        while (dstId >= intervals[exec_interval] && dstId < intervals[exec_interval+1] && hop < maxwalklength ){
            vid_t dstIdptr = dstId - intervals[exec_interval];
            eid_t outd = beg_pos[dstIdptr+1] - beg_pos[dstIdptr];
            if (outd > 0 && ((float)gpu_rand_r(&seed))/RAND_MAX > 0.15 ){
                eid_t pos = beg_pos[dstIdptr] + ((eid_t)gpu_rand_r(&seed))%outd;
                dstId = csr[pos];
            }else{
                reset = true;
                break;
            }
            hop++;
        }
        if( hop < maxwalklength && !reset ){
            sid_t p;
            for(p = 0; p < nshards; p++){
                if(dstId < intervals[p+1]) {
                    break;
                }
            }
            nowWalk = encode(sourId, dstId-intervals[p], hop);
            wid_t w = tid*(nshards*walkpitch) + p*walkpitch + tpnwalks[tid*nshards+p];
            if(tpnwalks[tid*nshards+p] < walkpitch-1) tpnwalks[tid*nshards+p]++;
            tpwalks[w] = nowWalk;
            // if(w == walkpitch+2) printf("pwalks[w] = %lld, \n", tpwalks[w]);
        }
        i += nthreads; //Next walk
    }
}
 
//int exec_update(RandomWalk &userprogram, Vertex *&vertices, WalkManager &walk_manager )
// int main(){
void exec_updates(metrics &m, eid_t *beg_pos, vid_t *csr, sid_t exec_interval, vid_t* intervals, WalkDataType* walks, WalkDataType **&pwalks, wid_t *&pnwalks, vid_t nverts, eid_t nedges, wid_t nwalks, sid_t nshards){
    struct timeval start, end;
    gettimeofday( &start, NULL );

    //define the variables used in GPU
    vid_t* d_intervals;
    eid_t* d_beg_pos;
    vid_t* d_csr;
    WalkDataType* d_walks; // walks in current interval copied to GPU
    WalkDataType* d_tpwalks; //walks moved to other intervals
    wid_t* d_tpnwalks;

    wid_t* tpnwalks;

    m.start_time("CPU_GPU_Memcpy");
    // std::cout << "before malloc device memory" << std::endl;
    //malloc device memory and copy data from host to device
    cudaMalloc((void**)&d_intervals, sizeof(vid_t) * (nshards+1));
    cudaMalloc((void**)&d_beg_pos, sizeof(eid_t) * (nverts+1));
    cudaMalloc((void**)&d_csr, sizeof(vid_t) * nedges);
    cudaMalloc((void**)&d_walks, sizeof(WalkDataType) * nwalks);

    cudaMemcpy(d_intervals, intervals, sizeof(vid_t)*(nshards+1), cudaMemcpyHostToDevice);
    cudaMemcpy(d_beg_pos, beg_pos, sizeof(eid_t)*(nverts+1), cudaMemcpyHostToDevice);
    cudaMemcpy(d_csr, csr, sizeof(vid_t)*nedges, cudaMemcpyHostToDevice);
    cudaMemcpy(d_walks, walks, sizeof(WalkDataType)*nwalks, cudaMemcpyHostToDevice);

    //malloc device memory for appended walks
    cudaMalloc((void**)&d_tpnwalks, sizeof(wid_t) * nthreads*nshards);
    cudaMemset(d_tpnwalks, 0, sizeof(wid_t) * nthreads*nshards);
    tpnwalks = (wid_t*)malloc(sizeof(wid_t) * nthreads*nshards);

    cudaMalloc((void**)&d_tpwalks, sizeof(WalkDataType) * nthreads*walkpitch*nshards);
    cudaMemset(d_tpwalks, 0, sizeof(WalkDataType) * nthreads*walkpitch*nshards);
    m.stop_time("CPU_GPU_Memcpy");

    // 定义kernel执行配置，28个block，每个block里面有1024个线程
    dim3 dimGrid(28);
    dim3 dimBlock(1024);
    
    // printf("exec_interval = %d, nverts = %d, nedges = %d \n", exec_interval, nverts, nedges);
    m.start_time("exec_updates in GPU");
    updatebywalk<<<dimGrid, dimBlock>>>(exec_interval, d_intervals, d_beg_pos, d_csr, d_walks, d_tpwalks, d_tpnwalks, nverts, nwalks, nshards);
    m.stop_time("exec_updates in GPU");

    m.start_time("CPU_GPU_Memcpy");
    // std::cout << "before cudaMemcpyDeviceToHost;" << std::endl;
    cudaMemcpy(tpnwalks, d_tpnwalks, sizeof(wid_t) * nthreads*nshards, cudaMemcpyDeviceToHost);
    for(sid_t p = 0; p < nshards; p++){
        pnwalks[p] = 0;
        for(sid_t t = 0; t < nthreads; t++){
            if(tpnwalks[t*nshards+p] >= walkpitch) printf("tpnwalks[%d][%d] = %d, pnwalks[%d] = %d, \n", t, p, tpnwalks[t*nshards+p], p, pnwalks[p]);
            pnwalks[p] += tpnwalks[t*nshards+p];
        }
        // printf("pnwalks[%d] = %d, \n", p, pnwalks[p]);
    }
    // 将device端数据拷贝到host端返回数据
    for(sid_t p = 0; p < nshards; p++){
        pwalks[p] = (WalkDataType*)malloc(sizeof(WalkDataType) * pnwalks[p]);
        unsigned off = 0;
        for(sid_t t = 0; t < nthreads; t++){
            cudaMemcpy(pwalks[p]+off, d_tpwalks + t*nshards*walkpitch+p*walkpitch, sizeof(WalkDataType) * tpnwalks[t*nshards+p], cudaMemcpyDeviceToHost);
            off += tpnwalks[t*nshards+p];
        }
    }
    m.stop_time("CPU_GPU_Memcpy");

    //释放设备内存
    // std::cout << "before cudaFree(d_intervals);" << std::endl;
    cudaFree(d_intervals);
    cudaFree(d_beg_pos);
    cudaFree(d_csr);
    cudaFree(d_walks);
    cudaFree(d_tpwalks);
    cudaFree(d_tpnwalks);
    free(tpnwalks);

    gettimeofday( &end, NULL );
    // int timeuse = 1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
    // printf("total time in exec_update is %d ms\n", timeuse/1000);
}
