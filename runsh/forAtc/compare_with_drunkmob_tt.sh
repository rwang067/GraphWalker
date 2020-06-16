#!/bin/bash

# 2019.1.5
# Twitter MSPPR
    echo "2019.1.5 observe the effect of shardsize for MSPPR in PC(dell)" >> graphwalker_metrics.txt.statistics 
    echo "app = MSPPR, dataset = Twitter" >> graphwalker_metrics.txt.statistics 

# numsources = 1, shardsize = 16M, walkspersource = 2000, firstsource = 13
echo "numsources = 1, shardsize = 16384(16MB), walkspersource = 2000, firstsource = 13" >> graphwalker_metrics.txt.statistics
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/msppr file  ../DataSet/Twitter/twitter_rv.net firstsource 13 numsources 1 shardsize 16384
done

# numsources = 10, shardsize = 16M, walkspersource = 2000, firstsource = 13
echo "numsources = 10, shardsize = 16384(16MB), walkspersource = 2000, firstsource = 13" >> graphwalker_metrics.txt.statistics
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/msppr file  ../DataSet/Twitter/twitter_rv.net firstsource 13 numsources 10 shardsize 16384
done

# numsources = 100, shardsize = 1024, walkspersource = 2000, firstsource = 13
echo "numsources = 100, shardsize = 16384(16MB), walkspersource = 2000, firstsource = 13" >> graphwalker_metrics.txt.statistics
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/msppr file  ../DataSet/Twitter/twitter_rv.net firstsource 13 numsources 100 shardsize 16384
done

# numsources = 1000, shardsize = 2048, walkspersource = 2000, firstsource = 13
echo "numsources = 1000, shardsize = 16384(16MB), walkspersource = 2000, firstsource = 13" >> graphwalker_metrics.txt.statistics
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/msppr file  ../DataSet/Twitter/twitter_rv.net firstsource 13 numsources 1000 shardsize 16384
done

# numsources = 10000, shardsize = 1048576, walkspersource = 2000, firstsource = 13
echo "numsources = 10000, shardsize = 16384(16MB), walkspersource = 2000, firstsource = 13" >> graphwalker_metrics.txt.statistics
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/msppr file  ../DataSet/Twitter/twitter_rv.net firstsource 13 numsources 10000 shardsize 16384
done

# numsources = 100000, shardsize = 1048576, walkspersource = 2000, firstsource = 13
echo "numsources = 100000, shardsize = 16384(16MB), walkspersource = 2000, firstsource = 13" >> graphwalker_metrics.txt.statistics
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/msppr file  ../DataSet/Twitter/twitter_rv.net firstsource 13 numsources 100000 shardsize 16384
done