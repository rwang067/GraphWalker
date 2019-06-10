echo "2019.6.10 " >> graphwalker_metrics.txt.statistics
echo "observe the impact of ngblocks in 8GB PC, app = msppr, dataset = Twitter" >> graphwalker_metrics.txt.statistics

### 8GB PC, SSD, Twitter
################################################################################################
for(( numsources = 10; numsources <= 1000; numsources*=10))
do
    echo "numsources = " $numsources >> graphwalker_metrics.txt.statistics
        echo "ngblocks = 1" >> graphwalker_metrics.txt.statistics
        for(( times = 0; times < 5; times++))
        do
            echo "times = " $times " from echo"
            sync; echo 1 > /proc/sys/vm/drop_caches
            ./bin/apps/msppr file ../dataset/Twitter/twitter_rv.net firstsource 12 numsources $numsources ngblocks 1
        done
    for(( ngblocks = 20; ngblocks <= 120; ngblocks+=20))
    do
        echo "ngblocks = " $ngblocks >> graphwalker_metrics.txt.statistics
        for(( times = 0; times < 5; times++))
        do
            echo "times = " $times " from echo"
            sync; echo 1 > /proc/sys/vm/drop_caches
            ./bin/apps/msppr file ../dataset/Twitter/twitter_rv.net firstsource 12 numsources $numsources ngblocks $ngblocks
        done
    done
done

### 8GB PC, SSD, Twitter
################################################################################################
sync; echo 1 > /proc/sys/vm/drop_caches
for(( numsources = 10; numsources <= 1000; numsources*=10))
do
    echo "numsources = " $numsources >> graphwalker_metrics.txt.statistics
        echo "ngblocks = 1" >> graphwalker_metrics.txt.statistics
        for(( times = 0; times < 5; times++))
        do
            echo "times = " $times " from echo"
            nocache ./bin/apps/msppr file ../dataset/Twitter/twitter_rv.net firstsource 12 numsources $numsources ngblocks 1
        done
    for(( ngblocks = 20; ngblocks <= 120; ngblocks+=20))
    do
        echo "ngblocks = " $ngblocks >> graphwalker_metrics.txt.statistics
        for(( times = 0; times < 5; times++))
        do
            echo "times = " $times " from echo"
            nocache ./bin/apps/msppr file ../dataset/Twitter/twitter_rv.net firstsource 12 numsources $numsources ngblocks $ngblocks
        done
    done
done