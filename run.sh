# !/bin/bash

# 2018.12.28
# Compare with GraphChi
echo "2018.12.28 Compare with GraphChi" >> graphchi_metrics.txt.statistics 
echo "app = RWD,GraphLet,SR,PPR dataset = Friendster" >> graphchi_metrics.txt.statistics 

# shardsize = 2048, SimRank, Friendster
echo "shardsize = 2048, App = SimRank, Dataset = Friendster" >> graphchi_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/simrank file ../DataSet/Friendster/out.friendster-reorder shardsize 2048 a 12 b 13
done

# shardsize = 2048, GraphLet, Friendster
echo "shardsize = 2048, App = GraphLet, Dataset = Friendster" >> graphchi_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/rwdomination file ../DataSet/Friendster/out.friendster-reorder shardsize 2048 nvertices 68349467
done

# shardsize = 2048, RWD, Friendster
echo "shardsize = 2048, App = RWD, Dataset = Friendster" >> graphchi_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    ./bin/apps/rwdomination file ../DataSet/Friendster/out.friendster-reorder shardsize 2048 nvertices 68349467
done
