echo "2019.6.14 " >> graphwalker_metrics.txt.statistics
echo "observe the impact of with and without pagecache (clear pagecache each time) in 64GB R730, app = msppr, dataset = Twitter" >> graphwalker_metrics.txt.statistics

### 64GB R730, SSD, Twitter
################################################################################################
for(( numsources = 1000; numsources <= 1000; numsources*=10))
do
    echo "numsources = " $numsources >> graphwalker_metrics.txt.statistics
    echo "Turn on Page Cache, nmblocks = 1" >> graphwalker_metrics.txt.statistics
    for(( times = 0; times < 3; times++))
    do
        echo "times = " $times " from echo"
        sync; echo 1 > /proc/sys/vm/drop_caches
        ./bin/apps/msppr file ../../raid0_mnop/Twitter/twitter_rv.net firstsource 12 numsources $numsources nmblocks 1
    done
    echo "Turn off Page Cache!" >> graphwalker_metrics.txt.statistics
    echo "Turn off Page Cache, nmblocks = 1"  >> graphwalker_metrics.txt.statistics
    for(( times = 0; times < 3; times++))
    do
        echo "times = " $times " from echo"
        sync; echo 1 > /proc/sys/vm/drop_caches
        /home/wang/nocache/nocache ./bin/apps/msppr file ../../raid0_mnop/Twitter/twitter_rv.net firstsource 12 numsources $numsources nmblocks 1
    done
    for(( nmblocks = 20; nmblocks <= 200; nmblocks+=20))
    do
        echo "Turn off Page Cache, nmblocks = " $nmblocks >> graphwalker_metrics.txt.statistics
        for(( times = 0; times < 3; times++))
        do
            echo "times = " $times " from echo"
            sync; echo 1 > /proc/sys/vm/drop_caches
            /home/wang/nocache/nocache ./bin/apps/msppr file ../../raid0_mnop/Twitter/twitter_rv.net firstsource 12 numsources $numsources nmblocks $nmblocks
        done
    done
done

echo "2019.6.14 " >> graphwalker_metrics.txt.statistics
echo "observe the impact of with and without pagecache (clear pagecache each time) in 64GB R730, app = msppr, dataset = Yahoo" >> graphwalker_metrics.txt.statistics

### 64GB R730, SSD, Yahoo
################################################################################################
for(( numsources = 1000; numsources <= 1000; numsources*=10))
do
    echo "numsources = " $numsources >> graphwalker_metrics.txt.statistics
    echo "Turn on Page Cache, nmblocks = 1" >> graphwalker_metrics.txt.statistics
    for(( times = 0; times < 3; times++))
    do
        echo "times = " $times " from echo"
        sync; echo 1 > /proc/sys/vm/drop_caches
        ./bin/apps/msppr file ../../raid0_mnop/Yahoo/yahoo-webmap.txt firstsource 4 numsources $numsources nmblocks 1
    done
    echo "Turn off Page Cache!" >> graphwalker_metrics.txt.statistics
    for(( times = 0; times < 3; times++))
    do
        echo "times = " $times " from echo"
        sync; echo 1 > /proc/sys/vm/drop_caches
        /home/wang/nocache/nocache ./bin/apps/msppr file ../../raid0_mnop/Yahoo/yahoo-webmap.txt firstsource 4 numsources $numsources nmblocks 1
    done
    for(( nmblocks = 100; nmblocks <= 900; nmblocks+=100))
    do
        echo "Turn off Page Cache, nmblocks = " $nmblocks >> graphwalker_metrics.txt.statistics
        for(( times = 0; times < 3; times++))
        do
            echo "times = " $times " from echo"
            sync; echo 1 > /proc/sys/vm/drop_caches
            /home/wang/nocache/nocache ./bin/apps/msppr file ../../raid0_mnop/Yahoo/yahoo-webmap.txt firstsource 4 numsources $numsources nmblocks $nmblocks
        done
    done
done

echo "2019.6.11 " >> graphwalker_metrics.txt.statistics
echo "observe the impact of with and without pagecache (clear pagecache each time) in 64GB R730, app = msppr, dataset = Crawl" >> graphwalker_metrics.txt.statistics

### 64GB R730, SSD, Crawl
################################################################################################
for(( numsources = 1000; numsources <= 1000; numsources*=10))
do
    echo "numsources = " $numsources >> graphwalker_metrics.txt.statistics
    echo "Turn on Page Cache, nmblocks = 1" >> graphwalker_metrics.txt.statistics
    for(( times = 0; times < 3; times++))
    do
        echo "times = " $times " from echo"
        sync; echo 1 > /proc/sys/vm/drop_caches
        ./bin/apps/msppr file ../../raid0_mnop/Crawl/crawl.txt firstsource 0 numsources $numsources nmblocks 1
    done
    echo "Turn off Page Cache!" >> graphwalker_metrics.txt.statistics
    for(( times = 0; times < 3; times++))
    do
        echo "times = " $times " from echo"
        sync; echo 1 > /proc/sys/vm/drop_caches
        /home/wang/nocache/nocache ./bin/apps/msppr file ../../raid0_mnop/Crawl/crawl.txt firstsource 0 numsources $numsources nmblocks 1
    done
    for(( nmblocks = 200; nmblocks <= 2600; nmblocks+=200))
    do
        echo "Turn off Page Cache, nmblocks = " $nmblocks >> graphwalker_metrics.txt.statistics
        for(( times = 0; times < 3; times++))
        do
            echo "times = " $times " from echo"
            sync; echo 1 > /proc/sys/vm/drop_caches
            /home/wang/nocache/nocache ./bin/apps/msppr file ../../raid0_mnop/Crawl/crawl.txt firstsource 0 numsources $numsources nmblocks $nmblocks
        done
    done
done

# echo "2019.5.15 " >> graphwalker_metrics.txt.statistics
# echo "observe the impact of nmblocks in 64GB R730, app = msppr, dataset = Crawl" >> graphwalker_metrics.txt.statistics

# ### 64GB R730, SSD, Crawl
# ################################################################################################
# for(( numsources = 1; numsources <= 10000; numsources*=10))
# do
#     echo "numsources = " $numsources >> graphwalker_metrics.txt.statistics
#     for(( nmblocks = 1; nmblocks <= 2048; nmblocks*=2))
#     do
#         echo "nmblocks = " $nmblocks >> graphwalker_metrics.txt.statistics
#         for(( times = 0; times < 5; times++))
#         do
#             echo "times = " $times " from echo"
#             ./bin/apps/msppr file ../../raid0_mnop/Crawl/crawl.txt firstsource 0 numsources $numsources nmblocks $nmblocks
#         done
#     done
# done

# echo "2019.5.19 " >> graphwalker_metrics.txt.statistics
# echo "observe the impact of nmblocks without pagecache in 8GB PC, app = msppr, dataset = Twitter" >> graphwalker_metrics.txt.statistics

# ### 8GB PC, SSD, Twitter
# ################################################################################################
# for(( numsources = 10; numsources <= 1000; numsources*=10))
# do
#     echo "numsources = " $numsources >> graphwalker_metrics.txt.statistics
#         echo "nmblocks = 1" >> graphwalker_metrics.txt.statistics
#         for(( times = 0; times < 5; times++))
#         do
#             echo "times = " $times " from echo"
#             nocache ./bin/apps/msppr file ../dataset/Twitter/twitter_rv.net firstsource 12 numsources $numsources nmblocks 1
#         done
#     for(( nmblocks = 20; nmblocks <= 120; nmblocks+=20))
#     do
#         echo "nmblocks = " $nmblocks >> graphwalker_metrics.txt.statistics
#         for(( times = 0; times < 5; times++))
#         do
#             echo "times = " $times " from echo"
#             nocache ./bin/apps/msppr file ../dataset/Twitter/twitter_rv.net firstsource 12 numsources $numsources nmblocks $nmblocks
#         done
#     done
# done