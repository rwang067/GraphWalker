echo "2019.7.12 " >> graphwalker_metrics.txt.statistics
echo "overall raw random walks performance under different R and L settings (clear pagecache each time) in 64GB R730, dataset = Yahoo" >> graphwalker_metrics.txt.statistics
echo "Turn on Page Cache, clear pagecache each time, blocksize = 2MB, nmblocks = 30000" >> graphwalker_metrics.txt.statistics

### 64GB R730, SSD, Yahoo
################################################################################################
echo "L = 10, different Rs" >> graphwalker_metrics.txt.statistics
for(( R = 1000000000; R <= 10000000000; R*=10))
do
    echo "L = 10, R = " $R >> graphwalker_metrics.txt.statistics
    echo -e "Runtime \t chooseBlock \t findSubGraph \t getCurrentWalks \t writeWalks2Disk \t exec_update \t updateWalkNum \t #exec_update \t #blocks" >> graphwalker_metrics.txt.statistics
    for(( times = 0; times < 3; times++))
    do
        echo "times = " $times " from echo"
        free -m
        sync; echo 1 > /proc/sys/vm/drop_caches
        free -m
        ./bin/apps/rawrandomwalks file ../../raid0_defghij/Yahoo/yahoo-webmap.txt N 1413511394 R $R L 10
    done
done

# ### 64GB R730, SSD, Yahoo
# ################################################################################################
# echo "R = 100000, different Ls" >> graphwalker_metrics.txt.statistics
# for(( L = 4; L <= 4096; L*=2))
# do
#     echo "R = 100000, L = " $L >> graphwalker_metrics.txt.statistics
#     echo -e "Runtime \t chooseBlock \t findSubGraph \t getCurrentWalks \t writeWalks2Disk \t exec_update \t updateWalkNum \t #exec_update \t #blocks" >> graphwalker_metrics.txt.statistics
#     for(( times = 0; times < 5; times++))
#     do
#         echo "times = " $times " from echo"
#         free -m
#         sync; echo 1 > /proc/sys/vm/drop_caches
#         free -m
#         ./bin/apps/rawrandomwalks file ../../raid0_defghij/Yahoo/yahoo-webmap.txt N 1413511394 R 100000 L $L
#     done
# done

echo "2019.7.12 " >> graphwalker_metrics.txt.statistics
echo "random walks applications performance comparison (clear pagecache each time) in 64GB R730, dataset = Crawl" >> graphwalker_metrics.txt.statistics
echo "Turn on Page Cache, clear pagecache each time" >> graphwalker_metrics.txt.statistics

### 64GB R730, SSD, Crawl
################################################################################################
echo "APP = PPR " >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file ../../raid0_defghij/Crawl/crawl.txt firstsource 0 numsources 1
done

### 64GB R730, SSD, Crawl
################################################################################################
echo "App = SimRank" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/simrank file ../../raid0_defghij/Crawl/crawl.txt a 0 b 1
done

### 64GB R730, SSD, Crawl
################################################################################################
echo "App = Graphlet" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/graphlet file ../../raid0_defghij/Crawl/crawl.txt N 3563602789 R 100000
done

## 64GB R730, SSD, Crawl
###############################################################################################
echo "App = RandomWalkDomination" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/rwdomination file ../../raid0_defghij/Crawl/crawl.txt N 3563602789
done

