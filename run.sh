# number of steps = 10
echo "app = msppr, dataset = Twitter(undirected), L=10, numsources 1 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo" >> graphwalker_metrics.txt.statistics 
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file ../../data/raid0_defghij_ssd/twitter_rv.net firstsource 12 nmblocks 1 
done

# echo "app = msppr, dataset = Twitter(undirected), L=10, numsources 1000 from echo" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/msppr file ../../data/raid0_defghij_ssd/twitter_rv.net firstsource 12 nmblocks 1 numsources 1000
# done

# echo "app = msppr, dataset = Twitter(undirected), L=10, numsources 1000000 from echo" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/msppr file ../../data/raid0_defghij_ssd/twitter_rv.net firstsource 12 nmblocks 1 numsources 1000000
# done


# echo "app = msppr, dataset = friendster, L=10, numsources 1 from echo" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/msppr file /home/kvgroup/ruiwang/data/raid0_defghij_ssd/Friendster/out.friendster-reorder firstsource 12 nmblocks 1 
# done

# echo "app = msppr, dataset = friendster, L=10, numsources 1000 from echo" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/msppr file /home/kvgroup/ruiwang/data/raid0_defghij_ssd/Friendster/out.friendster-reorder firstsource 12 nmblocks 1 numsources 1000
# done

# echo "app = msppr, dataset = friendster, L=10, numsources 1000000 from echo" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/msppr file /home/kvgroup/ruiwang/data/raid0_defghij_ssd/Friendster/out.friendster-reorder firstsource 12 nmblocks 1 numsources 1000000
# done


# # C = 0.15
# echo "app = msppr, dataset = Twitter(undirected), C=0.15, numsources 1 from echo" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/msppr file ../../data/raid0_defghij_ssd/twitter_rv.net maxwalklength 16384 firstsource 12 nmblocks 1 
# done

# echo "app = msppr, dataset = Twitter(undirected), C=0.15, numsources 1000 from echo" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/msppr file ../../data/raid0_defghij_ssd/twitter_rv.net maxwalklength 16384 firstsource 12 nmblocks 1 numsources 1000
# done

# echo "app = msppr, dataset = Twitter(undirected), C=0.15, numsources 1000000 from echo" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/msppr file ../../data/raid0_defghij_ssd/twitter_rv.net maxwalklength 16384 firstsource 12 nmblocks 1 numsources 1000000
# done



# echo "app = msppr, dataset = friendster, C=0.15, numsources 1 from echo" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/msppr file /home/kvgroup/ruiwang/data/raid0_defghij_ssd/Friendster/out.friendster-reorder maxwalklength 16384 firstsource 12 nmblocks 1
# done

# echo "app = msppr, dataset = friendster, C=0.15, numsources 1000 from echo" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/msppr file /home/kvgroup/ruiwang/data/raid0_defghij_ssd/Friendster/out.friendster-reorder maxwalklength 16384 firstsource 12 nmblocks 1 numsources 1000
# done

# echo "app = msppr, dataset = friendster, C=0.15, numsources 1000000 from echo" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/msppr file /home/kvgroup/ruiwang/data/raid0_defghij_ssd/Friendster/out.friendster-reorder maxwalklength 16384 firstsource 12 nmblocks 1 numsources 1000000
# done