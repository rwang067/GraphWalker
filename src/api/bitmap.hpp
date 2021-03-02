#ifndef DEF_BITMAP_HPP
#define DEF_BITMAP_HPP

#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <zlib.h>

class BitMap{

private:
    char *bitmap;
    int size;

public:
    BitMap(){
        bitmap = NULL;
        size = 0;
    }
    BitMap(int size){ // contractor, init the bitmap
        bitmap = NULL;
        bitmap = new char[size];
        if (bitmap == NULL) {
            logstream(LOG_FATAL) << "ErroR In BitMap Constractor!" << std::endl;
        }else{
            memset(bitmap, 0x0, size * sizeof(char));
            this->size = size;
        }
    }


    /*
     * set the index bit to 1;
     */
    inline int bitmapSet(int index){
        int addr = index/8;
        int addroffset = index%8;
        unsigned char temp = 0x1 << addroffset;
        if (addr > (size+1)) {
            return 0;
        }else{
            // std::cout << "before bitmapSet for " << index << ": bitmap[" << addr << "] = " << (int)bitmap[addr] << std::endl;
            bitmap[addr] |= temp;
            // std::cout << "after bitmapSet for " << index << ": bitmap[" << addr << "] = " << (int)bitmap[addr] << std::endl;
            return 1;
        }
    }

    /*
     * return if the index in bitmap is 1;
     */
    inline int bitmapGet(int index){
        int addr = index/8;
        int addroffset = index%8;
        unsigned char temp = 0x1 << addroffset;
        // std::cout << "bitmapGet for " << index << ": bitmap[" << addr << "] = " << (int)bitmap[addr] << std::endl;
        if (addr > (size + 1)) {
            return 0;
        }else{
            return (bitmap[addr] & temp) > 0 ? 1 : 0;
        }
    }

    /*
     * del the index from 1 to 0
     */
    inline int bitmapDel(int index){
        if (bitmapGet(index) == 0) {
            return 0;
        }
        int addr = index/8;
        int addroffset = index%8;
        unsigned char temp = 0x1 << addroffset;
        if (addr > (size + 1)) {
            return 0;
        }else{
            bitmap[addr] ^= temp;
            return 1;
        }
    }

    /*
     * Reset all indexex to 0
     */
    inline void bitmapReset(){
        memset(bitmap, 0x0, size * sizeof(char));
    }

};

#endif