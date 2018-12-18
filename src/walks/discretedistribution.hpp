#ifndef DEF_DISCRETE_DISTRIBUTION
#define DEF_DISCRETE_DISTRIBUTION

#include <queue>
#include "api/datatype.hpp"

struct IdCount {
    vid_t id;
    uint16_t count;

    IdCount(vid_t id, uint16_t count) {
        this->id = id;
        this->count = count;
    }

    bool operator<(const IdCount ic) const {
        return (count > ic.count);
    }
};

class DiscreteDistribution{

public:
	vid_t* ids;
	uint16_t* counts;
	unsigned size;
    unsigned capacity;

public:
	DiscreteDistribution(){
		size = 0;
        capacity = 10;
        ids = (vid_t*)malloc(capacity*sizeof(vid_t));
		counts = (uint16_t*)malloc(capacity*sizeof(uint16_t));
	}

    DiscreteDistribution(unsigned _capacity){
		size = 0;
        capacity = _capacity;
        ids = (vid_t*)malloc(capacity*sizeof(vid_t));
		counts = (uint16_t*)malloc(capacity*sizeof(uint16_t));
	}

	~DiscreteDistribution(){
		free(ids);
		free(counts);
	}

    void reserve(unsigned newcapacity){
		if( newcapacity < size ){
            logstream(LOG_WARNING) << "cannot reserve vector_w as capacity < size : " << newcapacity << " < " << size << std::endl;
            exit(-1);
        }
		capacity = newcapacity;
		// ids = (vid_t*)realloc(ids, capacity*sizeof(vid_t));
		// counts = (uint16_t*)realloc(counts, capacity*sizeof(uint16_t));
        vid_t *tmpids = (vid_t*)realloc(ids, capacity*sizeof(vid_t));
        if (tmpids == NULL)
            logstream(LOG_WARNING) << "cannot realloc ids as newcapacity, capacity , size : " << newcapacity << " , "<< capacity << " , " << size << std::endl;
        ids = tmpids;
        uint16_t *tmpcounts = (uint16_t*)realloc(counts, capacity*sizeof(uint16_t));
        if (tmpcounts == NULL)
            logstream(LOG_WARNING) << "cannot realloc counts as newcapacity, capacity , size : " << newcapacity << " , "<< capacity << " , " << size << std::endl;
        counts = tmpcounts;
	}

    void add(vid_t _id){
        for(unsigned i = 0; i < size; i++){
            if(ids[i] == _id){
                counts[i]++;
                return ;
            }
        }
        if(size == capacity)  reserve(2*capacity+1);
        ids[size] = _id;
        counts[size] = 1;
        size++;
    }

    DiscreteDistribution* filter(uint16_t mincount){
        if (mincount <= 1) {
            return this;
        }

        int toRemove = 0;
        for(unsigned i=0; i < size; i++) {
            toRemove += (counts[i] < mincount &&  counts[i] > 0 ? 1 : 0);
        }
        if (toRemove == 0) {
            return this;   // We can safely return same instance, as this is immutable
        }

        DiscreteDistribution *filteredDist = new DiscreteDistribution(size - toRemove);
        // filteredDist.reserve(size - toRemove);
        int idx = 0;
        for(unsigned i=0; i < size; i++) {
            if (counts[i] >= mincount) {
                filteredDist->ids[idx] = ids[i];
                filteredDist->counts[idx] = (uint16_t) (counts[i] - mincount + 1);
                idx++;
            }
        }
        //mark : need to free old dist??!!
        return filteredDist;
    }

    void getTop(unsigned ntop){
        std::priority_queue<IdCount> topDist;
        IdCount minIC(ids[0],counts[0]);
        for(unsigned i = 0; i < size; i++){
            if(i < ntop){
                topDist.push( IdCount(ids[i],counts[i]) );
                minIC = topDist.top();
            }else{
                if(minIC.count < counts[i]){
                    topDist.pop();
                    topDist.push(IdCount(ids[i],counts[i]));
                    minIC = topDist.top();
                }
            }
        }
        logstream(LOG_INFO) << "Top " << ntop << " visitfrequencies - " << std::endl;
        int i = 0;
        while(!topDist.empty()){
            IdCount ic = topDist.top();
            logstream(LOG_INFO) << i++ << "-\t" << ic.id << ":\t " << ic.count << std::endl;
            topDist.pop();
        }
    }
};

#endif