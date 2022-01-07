# msppr
################################################################################################
# Twitter
echo "app = msppr, dataset = Twitter, numsources 1 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file ../../data/raid0_defghij_ssd/twitter_rv.net firstsource 12 numsources 1 blocksize_kb 16384
done

# Friendster
echo "app = msppr, dataset = Friendster, numsources 1 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file ../../data/raid0_defghij_ssd/Friendster/out.friendster-reorder firstsource 0 numsources 1 blocksize_kb 2048
done

# Yahoo
echo "app = msppr, dataset = Yahoo, numsources 1 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Yahoo/yahoo-webmap.txt firstsource 0 numsources 1 blocksize_kb 2048
done

# Kron30
echo "app = msppr, dataset = Kron30, numsources 1 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Kron30/kron30_32-sorted.txt firstsource 0 numsources 1 blocksize_kb 131072
done

# Kron31
echo "app = msppr, dataset = Kron31, numsources 1 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Kron31/kron31_32-sorted.txt firstsource 0 numsources 1 blocksize_kb 2048
done

# Crawl
echo "app = msppr, dataset = Crawl, numsources 1 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/msppr file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Crawl/crawl.txt firstsource 0 numsources 1 blocksize_kb 2048
done

# simrank
################################################################################################
# Twitter
echo "app = simrank, dataset = Twitter from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/simrank file ../../data/raid0_defghij_ssd/twitter_rv.net blocksize_kb 16384 a 12 b 13
done

# Friendster
echo "app = simrank, dataset = Friendster from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/simrank file ../../data/raid0_defghij_ssd/Friendster/out.friendster-reorder blocksize_kb 2048 a 12 b 13
done

# Yahoo
echo "app = simrank, dataset = Yahoo from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/simrank file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Yahoo/yahoo-webmap.txt blocksize_kb 2048 a 4 b 9
done

# Kron30
echo "app = simrank, dataset = Kron30 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/simrank file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Kron30/kron30_32-sorted.txt blocksize_kb 131072 a 0 b 7
done

# Kron31
echo "app = simrank, dataset = Kron31 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/simrank file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Kron31/kron31_32-sorted.txt blocksize_kb 2048 a 0 b 6
done

# Crawl
echo "app = simrank, dataset = Crawl from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/simrank file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Crawl/crawl.txt blocksize_kb 2048 a 0 b 1
done

# graphlet
################################################################################################
# Twitter
echo "app = graphlet, dataset = Twitter from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/graphlet file ../../data/raid0_defghij_ssd/twitter_rv.net N 61578415 blocksize_kb 16384
done

# Friendster
echo "app = graphlet, dataset = Friendster from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/graphlet file ../../data/raid0_defghij_ssd/Friendster/out.friendster-reorder N 68349467 blocksize_kb 2048
done

# Yahoo
echo "app = graphlet, dataset = Yahoo from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/graphlet file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Yahoo/yahoo-webmap.txt N 1413511394 blocksize_kb 2048
done

# Kron30
echo "app = graphlet, dataset = Kron30 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 2; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/graphlet file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Kron30/kron30_32-sorted.txt N 1073741823 blocksize_kb 131072
done

# Kron31
echo "app = graphlet, dataset = Kron31 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 2; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/graphlet file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Kron31/kron31_32-sorted.txt N 2147483648 blocksize_kb 2048
done

# Crawl
echo "app = graphlet, dataset = Crawl from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 2; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/graphlet file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Crawl/crawl.txt N 3563602789 blocksize_kb 2048
done

# rwdomination
################################################################################################
# Twitter
echo "app = rwdomination, dataset = Twitter from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/rwdomination file ../../data/raid0_defghij_ssd/twitter_rv.net N 61578415 blocksize_kb 16384
done

# Friendster
echo "app = rwdomination, dataset = Friendster from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/rwdomination file ../../data/raid0_defghij_ssd/Friendster/out.friendster-reorder N 68349467 blocksize_kb 2048
done

# Yahoo
echo "app = rwdomination, dataset = Yahoo from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 2; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/rwdomination file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Yahoo/yahoo-webmap.txt N 1413511394 blocksize_kb 2048
done

# Kron30
echo "app = rwdomination, dataset = Kron30 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 2; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/rwdomination file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Kron30/kron30_32-sorted.txt N 1073741823 blocksize_kb 131072
done

# Kron31
echo "app = rwdomination, dataset = Kron31 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 2; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/rwdomination file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Kron31/kron31_32-sorted.txt N 2147483648 blocksize_kb 2048
done

# Crawl
echo "app = rwdomination, dataset = Crawl from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 2; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/rwdomination file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Crawl/crawl.txt N 3563602789 blocksize_kb 2048
done

mv graphwalker_metrics.txt.statistics graphwalker_metrics_fig18.txt
touch graphwalker_metrics.txt.statistics