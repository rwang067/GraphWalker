#ifndef DEF_WALK_BUFFER
#define DEF_WALK_BUFFER


#include <cstring>
#include "api/datatype.hpp"

class WalkBuffer{

public:
	wid_t size_w;
	WalkDataType *walks;

public:
	WalkBuffer(){
		size_w = 0;
		walks = (WalkDataType*)malloc(WALK_BUFFER_SIZE*sizeof(WalkDataType));
	}

	~WalkBuffer(){
		free(walks);
	}

    WalkDataType& operator[] (int i){
        return walks[i];
    }

	void push_back(WalkDataType w){
        walks[size_w++] = w;
	}
};

#endif