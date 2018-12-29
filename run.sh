# !/bin/bash

# 2018.12.28
# Compare with GraphChi
echo "2018.12.28 Compare with GraphChi" >> graphwalker_metrics.txt.statistics 
echo "app = RWD,GraphLet,SR,PPR dataset = Friendster" >> graphwalker_metrics.txt.statistics 

# ### HDD
# ################################################################################################
# ################################################################################################

# ## LJ
# ################################################################################################
# # shardsize = 128, PPR, LJ
# echo "HDD shardsize = 128, App = PPR, Dataset = LJ" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     ./bin/apps/personalizedpagerank file ../DataSet/LiveJournal/soc-LiveJournal1.txt nvertices 4847571 source 0 shardsize 128
# done

# # shardsize = 128, SimRank, LJ
# echo "HDD shardsize = 128, App = SimRank, Dataset = LJ" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     ./bin/apps/simrank file ../DataSet/LiveJournal/soc-LiveJournal1.txt a 0 b 1 shardsize 128
# done

# # shardsize = 1048576, RWD, LJ
# echo "HDD shardsize = 1048576, App = RWD, Dataset = LJ" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     ./bin/apps/rwdomination file ../DataSet/LiveJournal/soc-LiveJournal1.txt nvertices 4847571 shardsize 1048576
# done

# # shardsize = 1048576, Graphlet, LJ
# echo "HDD shardsize = 1048576, App = Graphlet, Dataset = LJ" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     ./bin/apps/graphlet file ../DataSet/LiveJournal/soc-LiveJournal1.txt nvertices 4847571 shardsize 1048576
# done

## WP
################################################################################################
# shardsize = 128, PPR, WP
echo "HDD shardsize = 128, App = PPR, Dataset = WP" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/personalizedpagerank file ../DataSet/Wikipedia/wikipedia_sorted.data nvertices 12150977 source 1 shardsize 128
done

# shardsize = 128, SimRank, WP
echo "HDD shardsize = 128, App = SimRank, Dataset = WP" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/simrank file ../DataSet/Wikipedia/wikipedia_sorted.data a 1 b 3 shardsize 128
done

# shardsize = 1048576, RWD, WP
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

## TT
################################################################################################
# shardsize = 128, PPR, TT
echo "HDD shardsize = 128, App = PPR, Dataset = TT" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/personalizedpagerank file ../DataSet/Twitter/twitter_rv.net nvertices 61578415 source 12 shardsize 128
done

# shardsize = 128, SimRank, TT
echo "HDD shardsize = 128, App = SimRank, Dataset = TT" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/simrank file ../DataSet/Twitter/twitter_rv.net a 12 b 13 shardsize 128
done

# shardsize = 1048576, RWD, TT
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

### SSD
################################################################################################
################################################################################################

## LJ
################################################################################################
# shardsize = 128, PPR, LJ
echo "HDD shardsize = 128, App = PPR, Dataset = LJ" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/personalizedpagerank file ../dataset /LiveJournal/soc-LiveJournal1.txt nvertices 4847571 source 0 shardsize 128
done

# shardsize = 128, SimRank, LJ
echo "HDD shardsize = 128, App = SimRank, Dataset = LJ" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/simrank file ../dataset /LiveJournal/soc-LiveJournal1.txt a 0 b 1 shardsize 128
done

# shardsize = 1048576, RWD, LJ
echo "HDD shardsize = 1048576, App = RWD, Dataset = LJ" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/rwdomination file ../dataset /LiveJournal/soc-LiveJournal1.txt nvertices 4847571 shardsize 1048576
done

# shardsize = 1048576, Graphlet, LJ
echo "HDD shardsize = 1048576, App = Graphlet, Dataset = LJ" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/graphlet file ../dataset /LiveJournal/soc-LiveJournal1.txt nvertices 4847571 shardsize 1048576
done

## WP
################################################################################################
# shardsize = 128, PPR, WP
echo "HDD shardsize = 128, App = PPR, Dataset = WP" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/personalizedpagerank file ../dataset /Wikipedia/wikipedia_sorted.data nvertices 12150977 source 1 shardsize 128
done

# shardsize = 128, SimRank, WP
echo "HDD shardsize = 128, App = SimRank, Dataset = WP" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/simrank file ../dataset /Wikipedia/wikipedia_sorted.data a 1 b 3 shardsize 128
done

# shardsize = 1048576, RWD, WP
echo "HDD shardsize = 1048576, App = RWD, Dataset = WP" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/rwdomination file ../dataset /Wikipedia/wikipedia_sorted.data nvertices 12150977 shardsize 1048576
done

# shardsize = 1048576, Graphlet, WP
echo "HDD shardsize = 1048576, App = Graphlet, Dataset = WP" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/graphlet file ../dataset /Wikipedia/wikipedia_sorted.data nvertices 12150977 shardsize 1048576
done

## TT
################################################################################################
# shardsize = 128, PPR, TT
echo "HDD shardsize = 128, App = PPR, Dataset = TT" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/personalizedpagerank file ../dataset /Twitter/twitter_rv.net nvertices 61578415 source 12 shardsize 128
done

# shardsize = 128, SimRank, TT
echo "HDD shardsize = 128, App = SimRank, Dataset = TT" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/simrank file ../dataset /Twitter/twitter_rv.net a 12 b 13 shardsize 128
done

# shardsize = 1048576, RWD, TT
echo "HDD shardsize = 1048576, App = RWD, Dataset = TT" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/rwdomination file ../dataset /Twitter/twitter_rv.net nvertices 61578415 shardsize 1048576
done

# shardsize = 1048576, Graphlet, TT
echo "HDD shardsize = 1048576, App = Graphlet, Dataset = TT" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/graphlet file ../dataset /Twitter/twitter_rv.net nvertices 61578415 shardsize 1048576
done



### shardsize for ppr
################################################################################################
################################################################################################

# # shardsize = 128K to 128M, PPR, LJ
# echo "numsources = 10000, walkspersource = 2000, firstsource = 12" >> graphwalker_metrics.txt.statistics 
# for(( shardsize = 256; shardsize <= 131072; shardsize*=2))
# do
#     echo "HDD shardsize = " $shardsize ", App = PPR, Dataset = LJ" >> graphwalker_metrics.txt.statistics 
#     for(( times = 0; times < 3; times++))
#     do
#         echo "times = " $times " from echo"
#         ./bin/apps/personalizedpagerank file ../DataSet/LiveJournal/soc-LiveJournal1.txt nvertices 4847571 source 0 shardsize $shardsize
#     done
# done

# shardsize = 128K to 128M, PPR, WP
echo "numsources = 10000, walkspersource = 2000, firstsource = 12" >> graphwalker_metrics.txt.statistics 
for(( shardsize = 256; shardsize <= 1048576; shardsize*=2))
do
    echo "HDD shardsize = " $shardsize ", App = PPR, Dataset = WP" >> graphwalker_metrics.txt.statistics 
    for(( times = 0; times < 3; times++))
    do
        echo "times = " $times " from echo"
        ./bin/apps/personalizedpagerank file ../DataSet/Wikipedia/wikipedia_sorted.data nvertices 12150977 source 1 shardsize $shardsize
    done
done

### MSPPR
################################################################################################
################################################################################################

# numsources = 10000, walkspersource = 2000, firstsource = 12
    echo "numsources = 10000, walkspersource = 2000, firstsource = 12" >> graphwalker_metrics.txt.statistics 
    for(( shardsize = 1024; shardsize <= 1048576; shardsize*=2))
    do
        echo "shardsize = " $shardsize >> graphwalker_metrics.txt.statistics 
        for(( numsources = 10000; numsources <= 10000; numsources*=10))
        do
            echo "shardsize = " $shardsize ", numsources = " $numsources >> graphwalker_metrics.txt.statistics 
            for(( times = 0; times < 5; times++))
            do
                echo "times = " $times " from echo"
                ./bin/apps/msppr file  ../DataSet/Friendster/out.friendster-reorder firstsource 12 numsources $numsources shardsize $shardsize
            done
        done
	done

### MSPPR numsources = 100000
echo "HDD shardsize = 1048576, App = MSPPR, firstsource = 12, numsources = 100000, Dataset = FS" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/msppr file ../DataSet/Friendster/out.friendster-reorder shardsize 1048576 firstsource 12 numsources 100000
done

# ### MSPPR #define KEEPWALKSINDISK 1 numsources = 100000
# echo "HDD shardsize = 1048576, App = MSPPR, firstsource = 12, numsources = 100000, Dataset = FS" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     ./bin/apps/msppr file ../DataSet/Friendster/out.friendster-reorder shardsize 1048576 firstsource 12 numsources 100000
# done
