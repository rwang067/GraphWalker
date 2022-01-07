### Twitter
################################################################################################
echo "dataset = Twitter, R = 100000, different Ls" >> graphwalker_metrics.txt.statistics
for(( L = 4; L <= 1024; L*=2))
do
    echo "R = 100000, L = " $L >> graphwalker_metrics.txt.statistics
    echo -e "Runtime \t chooseBlock \t findSubGraph \t getCurrentWalks \t writeWalks2Disk \t exec_update \t updateWalkNum \t #exec_update \t #blocks" >> graphwalker_metrics.txt.statistics
    for(( times = 0; times < 5; times++))
    do
        echo "times = " $times " from echo"
        free -m
        sync; echo 1 > /proc/sys/vm/drop_caches
        free -m
        ./bin/apps/rawrandomwalks file ../../data/raid0_defghij_ssd/twitter_rv.net blocksize_kb 16384 N 61578415 R 100000 L $L
    done
done

### Friendster
################################################################################################
echo "dataset = Friendster, R = 100000, different Ls" >> graphwalker_metrics.txt.statistics
for(( L = 4; L <= 1024; L*=2))
do
    echo "R = 100000, L = " $L >> graphwalker_metrics.txt.statistics
    echo -e "Runtime \t chooseBlock \t findSubGraph \t getCurrentWalks \t writeWalks2Disk \t exec_update \t updateWalkNum \t #exec_update \t #blocks" >> graphwalker_metrics.txt.statistics
    for(( times = 0; times < 5; times++))
    do
        echo "times = " $times " from echo"
        free -m
        sync; echo 1 > /proc/sys/vm/drop_caches
        free -m
        ./bin/apps/rawrandomwalks file ../../data/raid0_defghij_ssd/Friendster/out.friendster-reorder blocksize_kb 2048 N 68349467 R 100000 L $L
    done
done

### Yahoo
################################################################################################
echo "dataset = Yahoo, R = 100000, different Ls" >> graphwalker_metrics.txt.statistics
for(( L = 4; L <= 1024; L*=2))
do
    echo "R = 100000, L = " $L >> graphwalker_metrics.txt.statistics
    echo -e "Runtime \t chooseBlock \t findSubGraph \t getCurrentWalks \t writeWalks2Disk \t exec_update \t updateWalkNum \t #exec_update \t #blocks" >> graphwalker_metrics.txt.statistics
    for(( times = 0; times < 5; times++))
    do
        echo "times = " $times " from echo"
        free -m
        sync; echo 1 > /proc/sys/vm/drop_caches
        free -m
        ./bin/apps/rawrandomwalks file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Yahoo/yahoo-webmap.txt blocksize_kb 2048 N 1413511394 R 100000 L $L
    done
done

### Kron30
################################################################################################
echo "dataset = Kron30, R = 100000, different Ls" >> graphwalker_metrics.txt.statistics
for(( L = 4; L <= 1024; L*=2))
do
    echo "R = 100000, L = " $L >> graphwalker_metrics.txt.statistics
    echo -e "Runtime \t chooseBlock \t findSubGraph \t getCurrentWalks \t writeWalks2Disk \t exec_update \t updateWalkNum \t #exec_update \t #blocks" >> graphwalker_metrics.txt.statistics
    for(( times = 0; times < 2; times++))
    do
        echo "times = " $times " from echo"
        free -m
        sync; echo 1 > /proc/sys/vm/drop_caches
        free -m
        ./bin/apps/rawrandomwalks file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Kron30/kron30_32-sorted.txt blocksize_kb 131072 N 1073741823 R 100000 L $L
    done
done

mv graphwalker_metrics.txt.statistics graphwalker_metrics_fig17.txt
touch graphwalker_metrics.txt.statistics