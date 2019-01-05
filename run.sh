# 2019.1.4
# Run on Crawl in ccc
echo "2019.1.5" >> graphwalker_metrics.txt.statistics 
echo "app = compare with drunkardmob in Yahoo on 64GB ccc" >> graphwalker_metrics.txt.statistics 

### RAID
################################################################################################
################################################################################################

## Yahoo
################################################################################################
# shardsize = 1048576, RWD, Yahoo
echo "shardsize = 1024, RAID, App = MSPPR, Dataset = Yahoo" >> graphwalker_metrics.txt.statistics 
for(( numsources = 1; numsources <= 100000; numsources*=10))
do
    echo "numsources = " $numsources >> graphwalker_metrics.txt.statistics 
    for(( times = 0; times < 5; times++))
    do
        echo "times = " $times " from echo"
        ./bin/apps/personalizedpagerank file ../../raid0_efghij/Yahoo/yahoo-webmap.txt firstsource 9 shardsize 1024 numsources $numsources
    done
done