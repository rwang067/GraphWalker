echo "2019.7.1 " >> graphwalker_metrics.txt.statistics
echo "overall raw random walks performance under different R and L settings (clear pagecache each time) in 64GB R730, dataset = Yahoo" >> graphwalker_metrics.txt.statistics
echo "Turn on Page Cache, clear pagecache each time, blocksize = 2MB, nmblocks = 30000" >> graphwalker_metrics.txt.statistics

# ### 64GB R730, SSD, Yahoo
# ################################################################################################
# echo "L = 10, different Rs" >> graphwalker_metrics.txt.statistics
# for(( R = 1000; R <= 1000000000; R*=1000000))
# do
#     echo "L = 10, R = " $R >> graphwalker_metrics.txt.statistics
#     echo -e "Runtime \t chooseBlock \t findSubGraph \t getCurrentWalks \t writeWalks2Disk \t exec_update \t updateWalkNum \t #exec_update \t #blocks" >> graphwalker_metrics.txt.statistics
#     for(( times = 0; times < 10; times++))
#     do
#         echo "times = " $times " from echo"
#         free -m
#         sync; echo 1 > /proc/sys/vm/drop_caches
#         free -m
#         ./bin/apps/rawrandomwalks file ../../raid0_defghij/Yahoo/yahoo-webmap.txt blocksize_kb 2048 nmblocks 30000 N 1413511394 R $R L 10
#     done
# done

### 64GB R730, SSD, Yahoo
################################################################################################
echo "R = 100000, different Ls" >> graphwalker_metrics.txt.statistics
for(( L = 2048; L <= 16384; L*=2))
do
    echo "R = 100000, L = " $L >> graphwalker_metrics.txt.statistics
    echo -e "Runtime \t chooseBlock \t findSubGraph \t getCurrentWalks \t writeWalks2Disk \t exec_update \t updateWalkNum \t #exec_update \t #blocks" >> graphwalker_metrics.txt.statistics
    for(( times = 0; times < 5; times++))
    do
        echo "times = " $times " from echo"
        free -m
        sync; echo 1 > /proc/sys/vm/drop_caches
        free -m
        ./bin/apps/rawrandomwalks file ../../raid0_defghij/Yahoo/yahoo-webmap.txt blocksize_kb 2048 nmblocks 30000 N 1413511394 R 100000 L $L
    done
done
