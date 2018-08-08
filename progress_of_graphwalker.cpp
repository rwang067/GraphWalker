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