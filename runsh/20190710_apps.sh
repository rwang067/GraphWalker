echo "2019.7.6 " >> graphwalker_metrics.txt.statistics
echo "random walks applications performance comparison (clear pagecache each time) in 64GB R730, dataset = Twitter" >> graphwalker_metrics.txt.statistics
echo "Turn on Page Cache, clear pagecache each time, blocksize = 16MB, nmblocks = 3000" >> graphwalker_metrics.txt.statistics

# ### 64GB R730, SSD, Twitter
# ################################################################################################
# echo "APP = PPR " >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/msppr file ../../raid0_defghij/Twitter/twitter_rv.net blocksize_kb 16384 nmblocks 3000 firstsource 12 numsources 1
# done

# ### 64GB R730, SSD, Twitter
# ################################################################################################
# echo "App = SimRank" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/simrank file ../../raid0_defghij/Twitter/twitter_rv.net blocksize_kb 16384 nmblocks 3000 a 12 b 13
# done

# ### 64GB R730, SSD, Twitter
# ################################################################################################
# echo "App = Graphlet" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/graphlet file ../../raid0_defghij/Twitter/twitter_rv.net blocksize_kb 16384 nmblocks 3000 N 61578415 R 100000
# done

### 64GB R730, SSD, Twitter
################################################################################################
echo "App = RandomWalkDomination" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    # ./bin/apps/rwdomination file ../../raid0_defghij/Twitter/twitter_rv.net blocksize_kb 16384 nmblocks 3000 N 61578415
    ./bin/apps/rwdomination file ../../raid0_defghij/Twitter/twitter_rv.net blocksize_kb 524288 nmblocks 112 N 61578415
done



# echo "2019.7.6 " >> graphwalker_metrics.txt.statistics 
# echo "random walks applications performance comparison (clear pagecache each time) in 64GB R730, dataset = Friendster" >> graphwalker_metrics.txt.statistics 
# echo "Turn on Page Cache, clear pagecache each time, blocksize = 2MB, nmblocks = 30000" >> graphwalker_metrics.txt.statistics 

# ### 64GB R730, SSD, Friendster
# ################################################################################################
# echo "App = PPR" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/msppr file ../../raid0_defghij/Friendster/out.friendster-reorder blocksize_kb 2048 nmblocks 30000 firstsource 13
# done

# ### 64GB R730, SSD, Friendster
# ################################################################################################
# echo "App = SimRank" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/simrank file ../../raid0_defghij/Friendster/out.friendster-reorder blocksize_kb 2048 nmblocks 30000 a 12 b 13
# done

# ### 64GB R730, SSD, Friendster
# ################################################################################################
# echo "App = Graphlet" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     # ./bin/apps/graphlet file ../../raid0_defghij/Friendster/out.friendster-reorder blocksize_kb 2048 nmblocks 30000 N 68349467 R 100000
#     ./bin/apps/graphlet file ../../raid0_defghij/Friendster/out.friendster-reorder blocksize_kb 131072 nmblocks 450 N 68349467 R 100000
# done

### 64GB R730, SSD, Friendster
################################################################################################
# echo "App = RandomWalkDomination" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     # ./bin/apps/rwdomination file ../../raid0_defghij/Friendster/out.friendster-reorder blocksize_kb 2048 nmblocks 30000 N 68349467
#     ./bin/apps/rwdomination file ../../raid0_defghij/Friendster/out.friendster-reorder blocksize_kb 524288 nmblocks 112 N 68349467
# done



# echo "2019.7.6" >> graphwalker_metrics.txt.statistics 
# echo "random walks applications performance comparison (clear pagecache each time) in 64GB R730, dataset = Yahoo" >> graphwalker_metrics.txt.statistics 
# echo "Turn on Page Cache, clear pagecache each time, blocksize = 2MB, nmblocks = 30000" >> graphwalker_metrics.txt.statistics 

# ## 64GB R730, SSD, Yahoo
# ###############################################################################################
# echo "App = PPR" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/msppr file ../../raid0_defghij/Yahoo/yahoo-webmap.txt blocksize_kb 2048 nmblocks 30000 firstsource 9
# done

# ### 64GB R730, SSD, Yahoo
# ################################################################################################
# echo "App = SimRank" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/simrank file ../../raid0_defghij/Yahoo/yahoo-webmap.txt blocksize_kb 2048 nmblocks 30000 a 4 b 9
# done

### 64GB R730, SSD, Yahoo
################################################################################################
# echo "App = Graphlet" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     # ./bin/apps/graphlet file ../../raid0_defghij/Yahoo/yahoo-webmap.txt blocksize_kb 2048 nmblocks 30000 N 1413511394 R 100000
#     ./bin/apps/graphlet file ../../raid0_defghij/Yahoo/yahoo-webmap.txt blocksize_kb 131072 nmblocks 450 N 1413511394 R 100000
# done

# ### 64GB R730, SSD, Yahoo
# ################################################################################################
# echo "App = RandomWalkDomination" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     # ./bin/apps/rwdomination file ../../raid0_defghij/Yahoo/yahoo-webmap.txt blocksize_kb 2048 nmblocks 30000 N 1413511394
#     # ./bin/apps/rwdomination file ../../raid0_defghij/Yahoo/yahoo-webmap.txt blocksize_kb 32768 nmblocks 1800 N 1413511394
#     # ./bin/apps/rwdomination file ../../raid0_defghij/Yahoo/yahoo-webmap.txt blocksize_kb 262144 nmblocks 225 N 1413511394
#     # ./bin/apps/rwdomination file ../../raid0_defghij/Yahoo/yahoo-webmap.txt blocksize_kb 1048576 nmblocks 56 N 1413511394
#     ./bin/apps/rwdomination file ../../raid0_defghij/Yahoo/yahoo-webmap.txt blocksize_kb 4194304 nmblocks 14 N 1413511394
# done