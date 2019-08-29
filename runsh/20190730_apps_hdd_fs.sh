
echo "2019.7.30 " >> graphwalker_metrics.txt.statistics 
echo "random walks applications performance comparison (clear pagecache each time) in 64GB R730, dataset = Friendster, HDD" >> graphwalker_metrics.txt.statistics 
echo "Turn on Page Cache, clear pagecache each time" >> graphwalker_metrics.txt.statistics 

# ### 64GB R730, HDD, Friendster
# ################################################################################################
# echo "App = PPR" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/msppr file ../../sdl/Friendster/out.friendster-reorder firstsource 13
# done

### 64GB R730, HDD, Friendster
################################################################################################
echo "App = SimRank" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/simrank file ../../sdl/Friendster/out.friendster-reorder a 12 b 13
done

### 64GB R730, HDD, Friendster
################################################################################################
echo "App = Graphlet" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/graphlet file ../../sdl/Friendster/out.friendster-reorder N 68349467 R 100000
done

## 64GB R730, HDD, Friendster
###############################################################################################
echo "App = RandomWalkDomination" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/rwdomination file ../../sdl/Friendster/out.friendster-reorder N 68349467
done



echo "2019.7.30" >> graphwalker_metrics.txt.statistics 
echo "random walks applications performance comparison (clear pagecache each time) in 64GB R730, dataset = Yahoo, HDD" >> graphwalker_metrics.txt.statistics 
echo "Turn on Page Cache, clear pagecache each time" >> graphwalker_metrics.txt.statistics 

## 64GB R730, HDD, Yahoo
###############################################################################################
echo "App = PPR" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file ../../sdl/Yahoo/yahoo-webmap.txt firstsource 9
done

### 64GB R730, HDD, Yahoo
################################################################################################
echo "App = SimRank" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/simrank file ../../sdl/Yahoo/yahoo-webmap.txt a 4 b 9
done

## 64GB R730, HDD, Yahoo
###############################################################################################
echo "App = Graphlet" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/graphlet file ../../sdl/Yahoo/yahoo-webmap.txt N 1413511394 R 100000
done

### 64GB R730, HDD, Yahoo
################################################################################################
echo "App = RandomWalkDomination" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    # ./bin/apps/rwdomination file ../../sdl/Yahoo/yahoo-webmap.txt blocksize_kb 2048 nmblocks 30000 N 1413511394
    # ./bin/apps/rwdomination file ../../sdl/Yahoo/yahoo-webmap.txt blocksize_kb 32768 nmblocks 1800 N 1413511394
    # ./bin/apps/rwdomination file ../../sdl/Yahoo/yahoo-webmap.txt blocksize_kb 262144 nmblocks 225 N 1413511394
    # ./bin/apps/rwdomination file ../../sdl/Yahoo/yahoo-webmap.txt blocksize_kb 1048576 nmblocks 56 N 1413511394
    # ./bin/apps/rwdomination file ../../sdl/Yahoo/yahoo-webmap.txt blocksize_kb 4194304 nmblocks 14 N 1413511394
    ./bin/apps/rwdomination file ../../sdl/Yahoo/yahoo-webmap.txt N 1413511394
done