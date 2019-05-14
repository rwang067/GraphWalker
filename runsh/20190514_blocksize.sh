

echo "2019.5.9 " >> graphwalker_metrics.txt.statistics
echo "observe the impact of blocksize, app = msppr, dataset = Crawl" >> graphwalker_metrics.txt.statistics

### SSD, Crawl
################################################################################################
# shardsize = 1048576, RWD, Crawl
for(( numsources = 100000; numsources <= 100000; numsources*=10))
do
    echo "numsources = " $numsources >> graphwalker_metrics.txt.statistics
    for(( blocksize_kb = 32768; blocksize_kb <= 262144; blocksize_kb*=2))
    do
        echo "blocksize_kb = " $blocksize_kb >> graphwalker_metrics.txt.statistics
        for(( times = 0; times < 3; times++))
        do
            echo "times = " $times " from echo"
            ./bin/apps/msppr file ../../raid0_mnop/Crawl/crawl.txt firstsource 0 numsources $numsources blocksize_kb $blocksize_kb
        done
    done
done

# # 2019.5.7
# echo "2019.5.7" >> graphwalker_metrics.txt.statistics
# echo "observe the impact of blocksize, app = msppr, dataset = Yahoo" >> graphwalker_metrics.txt.statistics

# ### SSD, Yahoo
# ################################################################################################
# # shardsize = 1048576, RWD, Yahoo
# for(( numsources = 1; numsources <= 100000; numsources*=10))
# do
#     echo "numsources = " $numsources >> graphwalker_metrics.txt.statistics
#     for(( blocksize_kb = 2048; blocksize_kb <= 1048576*4; blocksize_kb*=2))
#     do
#         echo "blocksize_kb = " $blocksize_kb >> graphwalker_metrics.txt.statistics
#         for(( times = 0; times < 5; times++))
#         do
#             echo "times = " $times " from echo"
#             ./bin/apps/msppr file ../../raid0_mnop/Yahoo/yahoo-webmap.txt firstsource 4 numsources $numsources blocksize_kb $blocksize_kb
#         done
#     done
# done

# ### SSD, Friendster
# ################################################################################################
# # shardsize = 1048576, RWD, Friendster
# for(( numsources = 1; numsources <= 100000; numsources*=10))
# do
#     echo "numsources = " $numsources >> graphwalker_metrics.txt.statistics
#     for(( blocksize_kb = 1024; blocksize_kb <= 1048576*32; blocksize_kb*=2))
#     do
#         echo "blocksize_kb = " $blocksize_kb >> graphwalker_metrics.txt.statistics
#         for(( times = 0; times < 5; times++))
#         do
#             echo "times = " $times " from echo"
#             ./bin/apps/msppr file ../../raid0_mnop/Friendster/out.friendster-reorder firstsource 12 numsources $numsources blocksize_kb $blocksize_kb
#         done
#     done
# done