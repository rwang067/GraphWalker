# number of steps = 10
echo "app = msppr, dataset = Twitter, L=10, numsources 1 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file ../../data/raid0_defghij_ssd/twitter_rv.net firstsource 12 numsources 1 blocksize_kb 16384
done

echo "app = msppr, dataset = Twitter, L=10, numsources 100 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file ../../data/raid0_defghij_ssd/twitter_rv.net firstsource 12 numsources 100 blocksize_kb 16384
done

echo "app = msppr, dataset = Twitter, L=10, numsources 10000 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file ../../data/raid0_defghij_ssd/twitter_rv.net firstsource 12 numsources 10000 blocksize_kb 16384
done


echo "app = msppr, dataset = friendster, L=10, numsources 1 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file /home/kvgroup/ruiwang/data/raid0_defghij_ssd/Friendster/out.friendster-reorder firstsource 0 numsources 1 blocksize_kb 2048
done

echo "app = msppr, dataset = friendster, L=10, numsources 100 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file /home/kvgroup/ruiwang/data/raid0_defghij_ssd/Friendster/out.friendster-reorder firstsource 0 numsources 100 blocksize_kb 2048
done

echo "app = msppr, dataset = friendster, L=10, numsources 10000 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file /home/kvgroup/ruiwang/data/raid0_defghij_ssd/Friendster/out.friendster-reorder firstsource 0 numsources 10000 blocksize_kb 2048
done

echo "app = msppr, dataset = K30, L=10, numsources 1 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Kron30/kron30_32-sorted.txt numsources 1 firstsource 0 blocksize_kb 131072
done

echo "app = msppr, dataset = K30, L=10, numsources 100 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Kron30/kron30_32-sorted.txt numsources 100 firstsource 0 blocksize_kb 131072
done

echo "app = msppr, dataset = K30, L=10, numsources 10000 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Kron30/kron30_32-sorted.txt numsources 10000 firstsource 0 blocksize_kb 131072
done


# C = 0.15
echo "app = msppr, dataset = Twitter, C=0.15, numsources 1 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file ../../data/raid0_defghij_ssd/twitter_rv.net maxwalklength 16384 firstsource 12 numsources 1 blocksize_kb 16384
done 

echo "app = msppr, dataset = Twitter, C=0.15, numsources 100 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file ../../data/raid0_defghij_ssd/twitter_rv.net maxwalklength 16384 firstsource 12 numsources 100 blocksize_kb 16384
done

echo "app = msppr, dataset = Twitter, C=0.15, numsources 10000 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file ../../data/raid0_defghij_ssd/twitter_rv.net maxwalklength 16384 firstsource 12 numsources 10000 blocksize_kb 16384
done



echo "app = msppr, dataset = friendster, C=0.15, numsources 1 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file /home/kvgroup/ruiwang/data/raid0_defghij_ssd/Friendster/out.friendster-reorder maxwalklength 16384 firstsource 0 numsources 1 blocksize_kb 2048
done

echo "app = msppr, dataset = friendster, C=0.15, numsources 100 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file /home/kvgroup/ruiwang/data/raid0_defghij_ssd/Friendster/out.friendster-reorder maxwalklength 16384 firstsource 0 numsources 100 blocksize_kb 2048
done

echo "app = msppr, dataset = friendster, C=0.15, numsources 10000 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file /home/kvgroup/ruiwang/data/raid0_defghij_ssd/Friendster/out.friendster-reorder maxwalklength 16384 firstsource 0 numsources 10000 blocksize_kb 2048
done

echo "app = msppr, dataset = K30, C=0.15, numsources 1 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Kron30/kron30_32-sorted.txt numsources 1 firstsource 0 blocksize_kb 131072 maxwalklength 16384
done

echo "app = msppr, dataset = K30, C=0.15, numsources 100 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Kron30/kron30_32-sorted.txt numsources 100 firstsource 0 blocksize_kb 131072 maxwalklength 16384
done

echo "app = msppr, dataset = K30, C=0.15, numsources 10000 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Kron30/kron30_32-sorted.txt numsources 10000 firstsource 0 blocksize_kb 131072 maxwalklength 16384
done

# mv graphwalker_metrics.txt.statistics graphwalker_metrics_fig26.txt
# touch graphwalker_metrics.txt.statistics