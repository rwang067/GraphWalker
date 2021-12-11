/*
 * @Author: Long Deng
 * @Date: 2021-08-28 20:17:22
 * @LastEditors: Long Deng
 * @LastEditTime: 2021-08-28 21:23:03
 * @Description: file content
 */

#include <vector>

#include "api/datatype.hpp"

struct MemCSR {
    std::vector<size_t> index;
    std::vector<vid_t> data;

    struct CsrIterator {
        vid_t *pos;
        vid_t *end;

        CsrIterator(vid_t *pos, vid_t *end): pos(pos), end(end) {}

        bool valid() {}
        void next() {}
        vid_t out() {} // return dest vertex id
    };

    CsrIterator getNeighbors(vid_t v) {
        vid_t *data_addr = data.data();
        size_t pos = index[v];
        size_t end = index[v+1]; 
        return CsrIterator(data_addr + pos, data_addr + end);
    }



};
