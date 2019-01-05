# 2019.1.4
# Run on Crawl in ccc
echo "2019.1.4" >> graphwalker_metrics.txt.statistics 
echo "app = RWD,GraphLet,SR,PPR dataset = Crawl" >> graphwalker_metrics.txt.statistics 

### RAID
################################################################################################
################################################################################################

## Crawl
################################################################################################
# shardsize = 1048576, RWD, Crawl
echo "HDD shardsize = 1048576, App = RWD, Dataset = LJ" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/rwdomination ./bin/apps/simrank file ../../raid0_efghij/crawl.txt shardsize 1048576 nvertices 3563602788
done

# shardsize = 1048576, Graphlet, Crawl
echo "HDD shardsize = 1048576, App = Graphlet, Dataset = LJ" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/graphlet file ../../raid0_efghij/crawl.txt shardsize 1048576 nvertices 3563602788
done

# shardsize = 1048576, PPR, Crawl
echo "HDD shardsize = 64M(65536), App = PPR, Dataset = LJ" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/personalizedpagerank file ../../raid0_efghij/crawl.txt shardsize 1048576 nvertices 3563602788 source 1
done

# shardsize = 1048576, SimRank, Crawl
echo "HDD shardsize = 64M(65536), App = SimRank, Dataset = LJ" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/simrank file ../../raid0_efghij/crawl.txt shardsize 1048576 a 1 b 2
done