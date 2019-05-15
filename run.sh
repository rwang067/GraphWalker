

echo "2019.5.15 " >> graphwalker_metrics.txt.statistics
echo "observe the impact of nmblocks in 8GB PC, app = msppr, dataset = Twitter" >> graphwalker_metrics.txt.statistics

### 8GB PC, SSD, Twitter
################################################################################################
for(( numsources = 1; numsources <= 10000; numsources*=10))
do
    echo "numsources = " $numsources >> graphwalker_metrics.txt.statistics
        echo "nmblocks = 1" >> graphwalker_metrics.txt.statistics
        for(( times = 0; times < 5; times++))
        do
            echo "times = " $times " from echo"
            ./bin/apps/msppr file ../dataset/Twitter/twitter_rv.net firstsource 12 numsources $numsources nmblocks 1
        done
    for(( nmblocks = 20; nmblocks <= 120; nmblocks+=20))
    do
        echo "nmblocks = " $nmblocks >> graphwalker_metrics.txt.statistics
        for(( times = 0; times < 5; times++))
        do
            echo "times = " $times " from echo"
            ./bin/apps/msppr file ../dataset/Twitter/twitter_rv.net firstsource 12 numsources $numsources nmblocks $nmblocks
        done
    done
done