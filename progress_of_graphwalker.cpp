//created in 20180717
//mark the process and problems encountered in GraphWalker

1. make problems
    sudo apt install make
    sudo apt install g++
    problem : ./src/api/io.hpp:9:10: fatal error: zlib.h: No such file or directory
    solution : sudo apt-get install zlib1g-dev

2. dataset
    make sure the right address

3.Segmentation fault (core dumped)

4.  INFO:     graphwalker.hpp(run:192): numIntervals: 3 : 1
    terminate called after throwing an instance of 'std::bad_alloc'
    what():  std::bad_alloc
    Aborted (core dumped)

//modified in 20190521
TBD:
    1>. turn on PageCache, and set nmblocks = 1
    2>. turn off PageCache with nocache command, and set nmblocks as large as possible.
    the latter is a little bit of improvement upon the former.
    the exp results consult to : my work/exp/GraphWalker2.0/graphwalker.xlsx -- nmblocks--Crawl

Attention : 
    1>. With PageCache turning on, effects exist between differnt experiments.
    2>. open very large file cost a lot.(around 4s for a 500GB file)