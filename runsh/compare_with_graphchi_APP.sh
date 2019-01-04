# !/bin/bash

# 2018.12.28
# Compare with GraphChi
echo "2018.12.28 Compare with GraphChi" >> graphwalker_metrics.txt.statistics 
echo "app = RWD,GraphLet,SR,PPR dataset = Friendster" >> graphwalker_metrics.txt.statistics 

### HDD
################################################################################################
################################################################################################

## LJ
################################################################################################
# shardsize = 1048576, RWD, LJ
echo "HDD shardsize = 1048576, App = RWD, Dataset = LJ" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/rwdomination file ../DataSet/LiveJournal/soc-LiveJournal1.txt nvertices 4847571 shardsize 1048576
done

# shardsize = 1048576, Graphlet, LJ
echo "HDD shardsize = 1048576, App = Graphlet, Dataset = LJ" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/graphlet file ../DataSet/LiveJournal/soc-LiveJournal1.txt nvertices 4847571 shardsize 1048576
done

# shardsize = 64M(65536), PPR, LJ
echo "HDD shardsize = 64M(65536), App = PPR, Dataset = LJ" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/personalizedpagerank file ../DataSet/LiveJournal/soc-LiveJournal1.txt nvertices 4847571 source 0 shardsize 65536
done

# shardsize = 64M(65536), SimRank, LJ
echo "HDD shardsize = 64M(65536), App = SimRank, Dataset = LJ" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/simrank file ../DataSet/LiveJournal/soc-LiveJournal1.txt a 0 b 1 shardsize 65536
done

# WP
###############################################################################################
shardsize = 64M(65536), PPR, WP
shardsize = 1048576, RWD, WP
echo "HDD shardsize = 1048576, App = RWD, Dataset = WP" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/rwdomination file ../DataSet/Wikipedia/wikipedia_sorted.data nvertices 12150977 shardsize 1048576
done

# shardsize = 1048576, Graphlet, WP
echo "HDD shardsize = 1048576, App = Graphlet, Dataset = WP" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/graphlet file ../DataSet/Wikipedia/wikipedia_sorted.data nvertices 12150977 shardsize 1048576
done

echo "HDD shardsize = 64M(65536), App = PPR, Dataset = WP" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/personalizedpagerank file ../DataSet/Wikipedia/wikipedia_sorted.data nvertices 12150977 source 1 shardsize 65536
done

# shardsize = 64M(65536), SimRank, WP
echo "HDD shardsize = 64M(65536), App = SimRank, Dataset = WP" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/simrank file ../DataSet/Wikipedia/wikipedia_sorted.data a 1 b 3 shardsize 65536
done

# TT
###############################################################################################
shardsize = 64M(65536), PPR, TT
shardsize = 1048576, RWD, TT
echo "HDD shardsize = 1048576, App = RWD, Dataset = TT" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/rwdomination file ../DataSet/Twitter/twitter_rv.net nvertices 61578415 shardsize 1048576
done

# shardsize = 1048576, Graphlet, TT
echo "HDD shardsize = 1048576, App = Graphlet, Dataset = TT" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/graphlet file ../DataSet/Twitter/twitter_rv.net nvertices 61578415 shardsize 1048576
done

echo "HDD shardsize = 64M(65536), App = PPR, Dataset = TT" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/personalizedpagerank file ../DataSet/Twitter/twitter_rv.net nvertices 61578415 source 12 shardsize 65536
done

# shardsize = 64M(65536), SimRank, TT
echo "HDD shardsize = 64M(65536), App = SimRank, Dataset = TT" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/simrank file ../DataSet/Twitter/twitter_rv.net a 12 b 13 shardsize 65536
done

### SSD
################################################################################################
################################################################################################

## LJ
################################################################################################
# shardsize = 64M(65536), PPR, LJ
# shardsize = 1048576, RWD, LJ
echo "SSD shardsize = 1048576, App = RWD, Dataset = LJ" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/rwdomination file ../dataset/LiveJournal/soc-LiveJournal1.txt nvertices 4847571 shardsize 1048576
done

# shardsize = 1048576, Graphlet, LJ
echo "SSD shardsize = 1048576, App = Graphlet, Dataset = LJ" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/graphlet file ../dataset/LiveJournal/soc-LiveJournal1.txt nvertices 4847571 shardsize 1048576
done

echo "SSD shardsize = 64M(65536), App = PPR, Dataset = LJ" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/personalizedpagerank file ../dataset/LiveJournal/soc-LiveJournal1.txt nvertices 4847571 source 0 shardsize 65536
done

# shardsize = 65536, SimRank, LJ
echo "SSD shardsize = 65536, App = SimRank, Dataset = LJ" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/simrank file ../dataset/LiveJournal/soc-LiveJournal1.txt a 0 b 1 shardsize 65536
done

## WP
################################################################################################
# shardsize = 65536, PPR, WP
# shardsize = 1048576, RWD, WP
echo "SSD shardsize = 1048576, App = RWD, Dataset = WP" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/rwdomination file ../dataset/Wikipedia/wikipedia_sorted.data nvertices 12150977 shardsize 1048576
done

# shardsize = 1048576, Graphlet, WP
echo "SSD shardsize = 1048576, App = Graphlet, Dataset = WP" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/graphlet file ../dataset/Wikipedia/wikipedia_sorted.data nvertices 12150977 shardsize 1048576
done

echo "SSD shardsize = 65536, App = PPR, Dataset = WP" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/personalizedpagerank file ../dataset/Wikipedia/wikipedia_sorted.data nvertices 12150977 source 1 shardsize 65536
done

# shardsize = 65536, SimRank, WP
echo "SSD shardsize = 65536, App = SimRank, Dataset = WP" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/simrank file ../dataset/Wikipedia/wikipedia_sorted.data a 1 b 3 shardsize 65536
done

## TT
################################################################################################
# shardsize = 65536, PPR, TT
# shardsize = 1048576, RWD, TT
echo "SSD shardsize = 1048576, App = RWD, Dataset = TT" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/rwdomination file ../dataset/Twitter/twitter_rv.net nvertices 61578415 shardsize 1048576
done

# shardsize = 1048576, Graphlet, TT
echo "SSD shardsize = 1048576, App = Graphlet, Dataset = TT" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/graphlet file ../dataset/Twitter/twitter_rv.net nvertices 61578415 shardsize 1048576
done

echo "SSD shardsize = 65536, App = PPR, Dataset = TT" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/personalizedpagerank file ../dataset/Twitter/twitter_rv.net nvertices 61578415 source 12 shardsize 65536
done

# shardsize = 65536, SimRank, TT
echo "SSD shardsize = 65536, App = SimRank, Dataset = TT" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/simrank file ../dataset/Twitter/twitter_rv.net a 12 b 13 shardsize 65536
done