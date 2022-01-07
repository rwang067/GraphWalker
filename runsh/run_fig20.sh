### Friendster
################################################################################################
echo "dataset = Friendster, L = 10, different Rs" >> graphwalker_metrics.txt.statistics
for(( R = 1000; R <= 1000000000; R*=100))
do
    echo "L = 10, R = " $R >> graphwalker_metrics.txt.statistics
    echo -e "Runtime \t chooseBlock \t findSubGraph \t getCurrentWalks \t writeWalks2Disk \t exec_update \t updateWalkNum \t #exec_update \t #blocks" >> graphwalker_metrics.txt.statistics
    for(( times = 0; times < 5; times++))
    do
        echo "times = " $times " from echo"
        free -m
        sync; echo 1 > /proc/sys/vm/drop_caches
        free -m
        ./bin/apps/msppr file ../../data/raid0_defghij_ssd/Friendster/out.friendster-reorder blocksize_kb 2048 numsources 1 walkspersource $R
    done
done

### Crawl
################################################################################################
echo "dataset = Crawl, L = 10, different Rs" >> graphwalker_metrics.txt.statistics
for(( R = 1000; R <= 1000000000; R*=100))
do
    echo "L = 10, R = " $R >> graphwalker_metrics.txt.statistics
    echo -e "Runtime \t chooseBlock \t findSubGraph \t getCurrentWalks \t writeWalks2Disk \t exec_update \t updateWalkNum \t #exec_update \t #blocks" >> graphwalker_metrics.txt.statistics
    for(( times = 0; times < 2; times++))
    do
        echo "times = " $times " from echo"
        free -m
        sync; echo 1 > /proc/sys/vm/drop_caches
        free -m
        ./bin/apps/msppr file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Crawl/crawl.txt blocksize_kb 2048 numsources 1 walkspersource $R
    done
done

mv graphwalker_metrics.txt.statistics graphwalker_metrics_fig20.txt
touch graphwalker_metrics.txt.statistics