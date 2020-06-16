
#######################################################################################################################################
########################################################### nmblocks ################################################################
#######################################################################################################################################

# echo "2019.7.17 " >> graphwalker_metrics.txt.statistics
# echo "Impact of nmblocks in raw random walks (clear pagecache each time) in 64GB R730, dataset = Yahoo" >> graphwalker_metrics.txt.statistics
# echo "Turn on Page Cache, clear pagecache each time, R 100000 L 10" >> graphwalker_metrics.txt.statistics

# ### 64GB R730, SSD, Yahoo
# ################################################################################################
# echo "Turn on Page Cache, nmblocks = 1" >> graphwalker_metrics.txt.statistics
# echo "Turn on Page Cache, nmblocks = 1" >> graphwalker_metrics.txt.statistics
# echo -e "Runtime \t chooseBlock \t findSubGraph \t getCurrentWalks \t writeWalks2Disk \t exec_update \t updateWalkNum \t #exec_update \t #blocks_loading  #blocks" >> graphwalker_metrics.txt.statistics
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/rawrandomwalks file ../../raid0_defghij/Yahoo/yahoo-webmap.txt N 1413511394 R 100000 L 10 nmblocks 1
# done
# for(( nmblocks = 50; nmblocks <= 500; nmblocks+=50))
# do
#     echo "Turn on Page Cache, nmblocks = " $nmblocks >> graphwalker_metrics.txt.statistics
#     echo -e "Runtime \t chooseBlock \t findSubGraph \t getCurrentWalks \t writeWalks2Disk \t exec_update \t updateWalkNum \t #exec_update \t #blocks_loading  #blocks" >> graphwalker_metrics.txt.statistics
#     for(( times = 0; times < 5; times++))
#     do
#         echo "times = " $times " from echo"
#         free -m
#         sync; echo 1 > /proc/sys/vm/drop_caches
#         free -m
#         ./bin/apps/rawrandomwalks file ../../raid0_defghij/Yahoo/yahoo-webmap.txt N 1413511394 R 100000 L 10 nmblocks $nmblocks
#     done
# done

# echo "2019.7.17 " >> graphwalker_metrics.txt.statistics
# echo "Impact of nmblocks in raw random walks (clear pagecache each time) in 64GB R730, dataset = Crawl" >> graphwalker_metrics.txt.statistics
# echo "Turn on Page Cache, clear pagecache each time, R 100000 L 10 " >> graphwalker_metrics.txt.statistics

# ### 64GB R730, SSD, Crawl
# ################################################################################################
# echo "Turn on Page Cache, nmblocks = 1" >> graphwalker_metrics.txt.statistics
# echo -e "Runtime \t chooseBlock \t findSubGraph \t getCurrentWalks \t writeWalks2Disk \t exec_update \t updateWalkNum \t #exec_update \t #blocks_loading  #blocks" >> graphwalker_metrics.txt.statistics
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/rawrandomwalks file ../../raid0_defghij/Crawl/crawl.txt N 3563602789 R 100000 L 10 nmblocks 1
# done
# for(( nmblocks = 50; nmblocks <= 500; nmblocks+=50))
# do
#     echo "Turn on Page Cache, nmblocks = " $nmblocks >> graphwalker_metrics.txt.statistics
#     echo -e "Runtime \t chooseBlock \t findSubGraph \t getCurrentWalks \t writeWalks2Disk \t exec_update \t updateWalkNum \t #exec_update \t #blocks_loading  #blocks" >> graphwalker_metrics.txt.statistics
#     for(( times = 0; times < 5; times++))
#     do
#         echo "times = " $times " from echo"
#         free -m
#         sync; echo 1 > /proc/sys/vm/drop_caches
#         free -m
#         ./bin/apps/rawrandomwalks file ../../raid0_defghij/Crawl/crawl.txt N 3563602789 R 100000 L 10 nmblocks $nmblocks
#     done
# done

#######################################################################################################################################
########################################################### block size ################################################################
#######################################################################################################################################

# echo "2019.7.17 " >> graphwalker_metrics.txt.statistics
# echo "Impact of blocksize_kb in raw random walks (clear pagecache each time) in 64GB R730, dataset = Yahoo" >> graphwalker_metrics.txt.statistics
# echo "Turn on Page Cache, clear pagecache each time, R 100000 L 10" >> graphwalker_metrics.txt.statistics

# ### 64GB R730, SSD, Yahoo
# ################################################################################################
# for(( blocksize_kb = 2048; blocksize_kb <= 4194304; blocksize_kb*=2))
# do
#     echo "blocksize_kb = " $blocksize_kb >> graphwalker_metrics.txt.statistics
#     echo -e "Runtime \t chooseBlock \t findSubGraph \t getCurrentWalks \t writeWalks2Disk \t exec_update \t updateWalkNum \t #exec_update \t #blocks_loading  #blocks" >> graphwalker_metrics.txt.statistics
#     for(( times = 0; times < 5; times++))
#     do
#         echo "times = " $times " from echo"
#         free -m
#         sync; echo 1 > /proc/sys/vm/drop_caches
#         free -m
#         ./bin/apps/rawrandomwalks file ../../raid0_defghij/Yahoo/yahoo-webmap.txt N 1413511394 R 100000 L 10 nmblocks 1 blocksize_kb $blocksize_kb
#     done
# done

echo "2019.7.17 " >> graphwalker_metrics.txt.statistics
echo "Impact of blocksize_kb in raw random walks (clear pagecache each time) in 64GB R730, dataset = Crawl" >> graphwalker_metrics.txt.statistics
echo "Turn on Page Cache, clear pagecache each time, R 10000 L 10" >> graphwalker_metrics.txt.statistics

### 64GB R730, SSD, Crawl
################################################################################################
for(( blocksize_kb = 1024; blocksize_kb <= 1048576; blocksize_kb*=2))
do
    echo "blocksize_kb = " $blocksize_kb >> graphwalker_metrics.txt.statistics
    echo -e "Runtime \t chooseBlock \t findSubGraph \t getCurrentWalks \t exec_update \t updateWalkNum \t #exec_update \t #blocks_loading \t #blocks" >> graphwalker_metrics.txt.statistics
    for(( times = 0; times < 2; times++))
    do
        echo "times = " $times " from echo"
        free -m
        sync; echo 1 > /proc/sys/vm/drop_caches
        free -m
        ./bin/apps/rawrandomwalks file ../../raid0_defghij/Crawl/crawl.txt N 3563602789 R 10000 L 10 nmblocks 1 blocksize_kb $blocksize_kb
    done
done