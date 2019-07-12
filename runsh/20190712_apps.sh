echo "2019.7.10 " >> graphwalker_metrics.txt.statistics
echo "random walks applications performance comparison (clear pagecache each time) in 64GB R730, dataset = Twitter" >> graphwalker_metrics.txt.statistics
echo "Turn on Page Cache, clear pagecache each time" >> graphwalker_metrics.txt.statistics

### 64GB R730, SSD, Twitter
################################################################################################
echo "APP = PPR " >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file ../../raid0_defghij/Twitter/twitter_rv.net firstsource 12 numsources 1
done

### 64GB R730, SSD, Twitter
################################################################################################
echo "App = SimRank" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/simrank file ../../raid0_defghij/Twitter/twitter_rv.net a 12 b 13
done

### 64GB R730, SSD, Twitter
################################################################################################
echo "App = Graphlet" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/graphlet file ../../raid0_defghij/Twitter/twitter_rv.net N 61578415 R 100000
done

## 64GB R730, SSD, Twitter
###############################################################################################
echo "App = RandomWalkDomination" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/rwdomination file ../../raid0_defghij/Twitter/twitter_rv.net N 61578415
done



echo "2019.7.10 " >> graphwalker_metrics.txt.statistics 
echo "random walks applications performance comparison (clear pagecache each time) in 64GB R730, dataset = Friendster" >> graphwalker_metrics.txt.statistics 
echo "Turn on Page Cache, clear pagecache each time" >> graphwalker_metrics.txt.statistics 

### 64GB R730, SSD, Friendster
################################################################################################
echo "App = PPR" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file ../../raid0_defghij/Friendster/out.friendster-reorder firstsource 13
done

### 64GB R730, SSD, Friendster
################################################################################################
echo "App = SimRank" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/simrank file ../../raid0_defghij/Friendster/out.friendster-reorder a 12 b 13
done

### 64GB R730, SSD, Friendster
################################################################################################
echo "App = Graphlet" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/graphlet file ../../raid0_defghij/Friendster/out.friendster-reorder N 68349467 R 100000
done

## 64GB R730, SSD, Friendster
###############################################################################################
echo "App = RandomWalkDomination" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/rwdomination file ../../raid0_defghij/Friendster/out.friendster-reorder N 68349467
done



echo "2019.7.10" >> graphwalker_metrics.txt.statistics 
echo "random walks applications performance comparison (clear pagecache each time) in 64GB R730, dataset = Yahoo" >> graphwalker_metrics.txt.statistics 
echo "Turn on Page Cache, clear pagecache each time" >> graphwalker_metrics.txt.statistics 

## 64GB R730, SSD, Yahoo
###############################################################################################
echo "App = PPR" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file ../../raid0_defghij/Yahoo/yahoo-webmap.txt firstsource 9
done

### 64GB R730, SSD, Yahoo
################################################################################################
echo "App = SimRank" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/simrank file ../../raid0_defghij/Yahoo/yahoo-webmap.txt a 4 b 9
done

## 64GB R730, SSD, Yahoo
###############################################################################################
echo "App = Graphlet" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/graphlet file ../../raid0_defghij/Yahoo/yahoo-webmap.txt N 1413511394 R 100000
done

### 64GB R730, SSD, Yahoo
################################################################################################
echo "App = RandomWalkDomination" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    # ./bin/apps/rwdomination file ../../raid0_defghij/Yahoo/yahoo-webmap.txt blocksize_kb 2048 nmblocks 30000 N 1413511394
    # ./bin/apps/rwdomination file ../../raid0_defghij/Yahoo/yahoo-webmap.txt blocksize_kb 32768 nmblocks 1800 N 1413511394
    # ./bin/apps/rwdomination file ../../raid0_defghij/Yahoo/yahoo-webmap.txt blocksize_kb 262144 nmblocks 225 N 1413511394
    # ./bin/apps/rwdomination file ../../raid0_defghij/Yahoo/yahoo-webmap.txt blocksize_kb 1048576 nmblocks 56 N 1413511394
    # ./bin/apps/rwdomination file ../../raid0_defghij/Yahoo/yahoo-webmap.txt blocksize_kb 4194304 nmblocks 14 N 1413511394
    ./bin/apps/rwdomination file ../../raid0_defghij/Yahoo/yahoo-webmap.txt N 1413511394
done