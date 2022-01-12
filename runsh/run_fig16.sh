### Twitter
################################################################################################
echo "dataset = Twitter, L = 10, different Rs" >> graphwalker_metrics.txt.statistics
for(( R = 1000; R <= 100000000; R*=10))
do
    echo "L = 10, R = " $R >> graphwalker_metrics.txt.statistics
    echo -e "Runtime \t chooseBlock \t findSubGraph \t getCurrentWalks \t writeWalks2Disk \t exec_update \t updateWalkNum \t #exec_update \t #blocks" >> graphwalker_metrics.txt.statistics
    for(( times = 0; times < 5; times++))
    do
        echo "times = " $times " from echo"
        free -m
        sync; echo 1 > /proc/sys/vm/drop_caches
        free -m
        ./bin/apps/rawrandomwalks file ../../data/raid0_defghij_ssd/twitter_rv.net blocksize_kb 16384 N 61578415 R $R L 10
    done
done

### Friendster
################################################################################################
echo "dataset = Friendster, L = 10, different Rs" >> graphwalker_metrics.txt.statistics
for(( R = 1000; R <= 100000000; R*=10))
do
    echo "L = 10, R = " $R >> graphwalker_metrics.txt.statistics
    echo -e "Runtime \t chooseBlock \t findSubGraph \t getCurrentWalks \t writeWalks2Disk \t exec_update \t updateWalkNum \t #exec_update \t #blocks" >> graphwalker_metrics.txt.statistics
    for(( times = 0; times < 5; times++))
    do
        echo "times = " $times " from echo"
        free -m
        sync; echo 1 > /proc/sys/vm/drop_caches
        free -m
        ./bin/apps/rawrandomwalks file ../../data/raid0_defghij_ssd/Friendster/out.friendster-reorder blocksize_kb 2048 N 68349467 R $R L 10
    done
done

### Yahoo
################################################################################################
echo "dataset = Yahoo, L = 10, different Rs" >> graphwalker_metrics.txt.statistics
for(( R = 1000; R <= 10000000000; R*=10))
do
    echo "L = 10, R = " $R >> graphwalker_metrics.txt.statistics
    echo -e "Runtime \t chooseBlock \t findSubGraph \t getCurrentWalks \t writeWalks2Disk \t exec_update \t updateWalkNum \t #exec_update \t #blocks" >> graphwalker_metrics.txt.statistics
    for(( times = 0; times < 2; times++))
    do
        echo "times = " $times " from echo"
        free -m
        sync; echo 1 > /proc/sys/vm/drop_caches
        free -m
        ./bin/apps/rawrandomwalks file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Yahoo/yahoo-webmap.txt blocksize_kb 2048 N 1413511394 R $R L 10
    done
done

### Kron30
################################################################################################
echo "dataset = Kron30, L = 10, different Rs" >> graphwalker_metrics.txt.statistics
for(( R = 1000; R <= 10000000000; R*=10))
do
    echo "L = 10, R = " $R >> graphwalker_metrics.txt.statistics
    echo -e "Runtime \t chooseBlock \t findSubGraph \t getCurrentWalks \t writeWalks2Disk \t exec_update \t updateWalkNum \t #exec_update \t #blocks" >> graphwalker_metrics.txt.statistics
    for(( times = 0; times < 2; times++))
    do
        echo "times = " $times " from echo"
        free -m
        sync; echo 1 > /proc/sys/vm/drop_caches
        free -m
        ./bin/apps/rawrandomwalks file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Kron30/kron30_32-sorted.txt blocksize_kb 131072 N 1073741823 R $R L 10
    done
done

mv graphwalker_metrics.txt.statistics graphwalker_metrics_fig16.txt
touch graphwalker_metrics.txt.statistics