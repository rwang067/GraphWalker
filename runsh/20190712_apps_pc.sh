# echo "2019.7.12 " >> graphwalker_metrics.txt.statistics
# echo "random walks applications performance comparison (clear pagecache each time) in 8GB Dell PC, dataset = Wiki" >> graphwalker_metrics.txt.statistics
# echo "Turn on Page Cache, clear pagecache each time" >> graphwalker_metrics.txt.statistics

# ### 8GB PC, SSD, Wiki
# ################################################################################################
# echo "APP = PPR " >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/msppr file ../dataset/Wikipedia/wikipedia_sorted.data firstsource 3 numsources 1
# done

# ### 8GB PC, SSD, Wiki
# ################################################################################################
# echo "App = SimRank" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/simrank file ../dataset/Wikipedia/wikipedia_sorted.data a 1 b 3
# done

# ### 8GB PC, SSD, Wiki
# ################################################################################################
# echo "App = Graphlet" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/graphlet file ../dataset/Wikipedia/wikipedia_sorted.data N 12150977 R 100000
# done

# ## 8GB PC, SSD, Wiki
# ###############################################################################################
# echo "App = RandomWalkDomination" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/rwdomination file ../dataset/Wikipedia/wikipedia_sorted.data N 12150977
# done

echo "2019.7.12 " >> graphwalker_metrics.txt.statistics
echo "random walks applications performance comparison (clear pagecache each time) in 8GB Dell PC, dataset = Twitter" >> graphwalker_metrics.txt.statistics
echo "Turn on Page Cache, clear pagecache each time" >> graphwalker_metrics.txt.statistics

### 8GB PC, SSD, Twitter
################################################################################################
echo "APP = PPR " >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file ../dataset/Twitter/twitter_rv.net firstsource 12 numsources 1
done

### 8GB PC, SSD, Twitter
################################################################################################
echo "App = SimRank" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/simrank file ../dataset/Twitter/twitter_rv.net a 12 b 13
done

### 8GB PC, SSD, Twitter
################################################################################################
echo "App = Graphlet" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/graphlet file ../dataset/Twitter/twitter_rv.net N 61578415 R 100000
done

## 8GB PC, SSD, Twitter
###############################################################################################
echo "App = RandomWalkDomination" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/rwdomination file ../dataset/Twitter/twitter_rv.net N 61578415
done