echo "2019.6.14 " >> graphwalker_metrics.txt.statistics
echo "observe the impact of nmblocks (clear pagecache each time) in 64GB R730, app = msppr, dataset = Twitter" >> graphwalker_metrics.txt.statistics

# ### 64GB R730, SSD, Twitter
# ################################################################################################
# for(( numsources = 1000; numsources <= 1000; numsources*=10))
# do
#     echo "numsources = " $numsources >> graphwalker_metrics.txt.statistics
#     # echo "Turn on Page Cache, nmblocks = 1" >> graphwalker_metrics.txt.statistics
#     # for(( times = 0; times < 3; times++))
#     # do
#     #     echo "times = " $times " from echo"
#     #     sync; echo 1 > /proc/sys/vm/drop_caches
#     #     ./bin/apps/msppr file ../../raid0_mnop/Twitter/twitter_rv.net firstsource 12 numsources $numsources nmblocks 1
#     # done
#     for(( nmblocks = 40; nmblocks <= 200; nmblocks+=20))
#     do
#         echo "Turn on Page Cache, nmblocks = " $nmblocks >> graphwalker_metrics.txt.statistics
#         for(( times = 0; times < 3; times++))
#         do
#             echo "times = " $times " from echo"
#             free -m
#             sync; echo 1 > /proc/sys/vm/drop_caches
#             free -m
#             ./bin/apps/msppr file ../../raid0_mnop/Twitter/twitter_rv.net firstsource 12 numsources $numsources nmblocks $nmblocks
#         done
#     done
# done

# echo "2019.6.14 " >> graphwalker_metrics.txt.statistics
# echo "observe the impact of nmblocks (clear pagecache each time) in 64GB R730, app = msppr, dataset = Yahoo" >> graphwalker_metrics.txt.statistics

### 64GB R730, SSD, Yahoo
################################################################################################
for(( numsources = 1000; numsources <= 1000; numsources*=10))
do
    echo "numsources = " $numsources >> graphwalker_metrics.txt.statistics
    echo "Turn on Page Cache, nmblocks = 1" >> graphwalker_metrics.txt.statistics
    for(( times = 0; times < 10; times++))
    do
        echo "times = " $times " from echo"
        free -m
        sync; echo 1 > /proc/sys/vm/drop_caches
        free -m
        ./bin/apps/msppr file ../../raid0_mnop/Yahoo/yahoo-webmap.txt firstsource 4 numsources $numsources nmblocks 1
    done
    for(( nmblocks = 100; nmblocks <= 900; nmblocks+=100))
    do
        echo "Turn on Page Cache, nmblocks = " $nmblocks >> graphwalker_metrics.txt.statistics
        for(( times = 0; times < 10; times++))
        do
            echo "times = " $times " from echo"
            free -m
            sync; echo 1 > /proc/sys/vm/drop_caches
            free -m
            ./bin/apps/msppr file ../../raid0_mnop/Yahoo/yahoo-webmap.txt firstsource 4 numsources $numsources nmblocks $nmblocks
        done
    done
done

echo "2019.6.11 " >> graphwalker_metrics.txt.statistics
echo "observe the impact of nmblocks (clear pagecache each time) in 64GB R730, app = msppr, dataset = Crawl" >> graphwalker_metrics.txt.statistics

### 64GB R730, SSD, Crawl
################################################################################################
for(( numsources = 1000; numsources <= 1000; numsources*=10))
do
    echo "numsources = " $numsources >> graphwalker_metrics.txt.statistics
    echo "Turn on Page Cache, nmblocks = 1" >> graphwalker_metrics.txt.statistics
    for(( times = 0; times < 10; times++))
    do
        echo "times = " $times " from echo"
        free -m
        sync; echo 1 > /proc/sys/vm/drop_caches
        free -m
        ./bin/apps/msppr file ../../raid0_mnop/Crawl/crawl.txt firstsource 0 numsources $numsources nmblocks 1
    done
    for(( nmblocks = 200; nmblocks <= 2400; nmblocks+=200))
    do
        echo "Turn on Page Cache, nmblocks = " $nmblocks >> graphwalker_metrics.txt.statistics
        for(( times = 0; times < 10; times++))
        do
            echo "times = " $times " from echo"
            free -m
            sync; echo 1 > /proc/sys/vm/drop_caches
            free -m
            ./bin/apps/msppr file ../../raid0_mnop/Crawl/crawl.txt firstsource 0 numsources $numsources nmblocks $nmblocks
        done
    done
done