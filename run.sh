# 2019.5.7
echo "2019.5.7" >> graphwalker_metrics.txt.statistics
echo "observe the impact of blocksize, app = msppr" >> graphwalker_metrics.txt.statistics

### SSD, Yahoo
################################################################################################
# shardsize = 1048576, RWD, Yahoo
for(( numsources = 1; numsources <= 100000; numsources*=10))
do
    echo "numsources = " $numsources >> graphwalker_metrics.txt.statistics
    for(( blocksize_kb = 2048; blocksize_kb <= 1048576*32; blocksize_kb*=2))
    do
        echo "blocksize_kb = " $blocksize_kb >> graphwalker_metrics.txt.statistics
        for(( times = 0; times < 5; times++))
        do
            echo "times = " $times " from echo"
            ./bin/apps/msppr file ../../raid0_mnop/Yahoo/yahoo-webmap.txt firstsource 4 numsources $numsources blocksize_kb $blocksize_kb
        done
    done
done

# ### SSD, Friendster
# ################################################################################################
# # shardsize = 1048576, RWD, Yahoo
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