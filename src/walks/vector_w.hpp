#ifndef DEF_VECTOR_W
#define DEF_VECTOR_W


#include <cstring>
#include "api/datatype.hpp"

#define init_capacity 5

class VECTOR_W{

public:
	int size_w;
	int capacity_w;
	WalkDataType *walks;

public:
	VECTOR_W(){
        // logstream(LOG_WARNING) << " create vector_w " << std::endl;
		capacity_w = init_capacity;
		size_w = 0;
		walks = (WalkDataType*)malloc(capacity_w*sizeof(WalkDataType));
	}

	~VECTOR_W(){
		free(walks);
	}

    WalkDataType& operator[] (int i){
        return walks[i];
    }

	void reserve(int newcapacity){
        // logstream(LOG_WARNING) << " newcapacity, capacity , size : " << newcapacity << " , "<< capacity_w << " , " << size_w << std::endl;
		if( newcapacity < size_w ){
            logstream(LOG_WARNING) << "cannot reserve vector_w as capacity < size : " << newcapacity << " < " << size_w << std::endl;
            exit(-1);
        }
		capacity_w = newcapacity;
		// walks = (WalkDataType*)realloc(walks, capacity_w*sizeof(WalkDataType));
        WalkDataType* tmp = (WalkDataType*)realloc(walks, capacity_w*sizeof(WalkDataType));
        if (tmp == NULL) {
            // do something to deal with the problem
            logstream(LOG_WARNING) << "cannot realloc vector_w as newcapacity, capacity , size : " << newcapacity << " , "<< capacity_w << " , " << size_w << std::endl;
        }
        walks = tmp; // safe to use walks
	}

    void resize(int newsize){
        size_w = newsize;
        if(size_w > capacity_w)
            reserve(2*size_w+1);
    }

    void clear(){
        // free(walks);
        // walks = NULL;
        // capacity_w = init_capacity;
		size_w = 0;
		// walks = (WalkDataType*)malloc(capacity_w*sizeof(WalkDataType));
    }

	void push_back(WalkDataType w){
		if(size_w==capacity_w)
			reserve(1.5*capacity_w+1);
        else if(size_w > capacity_w) 
            logstream(LOG_ERROR) << "size_w, capacity_w : " << size_w << " , " << capacity_w << std::endl;
        walks[size_w++] = w;
	}

    void joint(VECTOR_W &append){
        this->capacity_w = this->size_w + append.size();
        walks = (WalkDataType*)realloc(walks, capacity_w*sizeof(WalkDataType));
        for(int i = 0; i < append.size(); i++)
            this->walks[size_w++] = append.walks[i];
        assert(this->size_w == this->capacity_w);
    }

    bool isEmpty(){
        return (size_w == 0);
    }

    int capacity() const{
        return capacity_w;
    }

    int size() const{
        return size_w;
    }
};

#endif