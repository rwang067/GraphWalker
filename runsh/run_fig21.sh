# msppr -- kk_ppr 
echo "app = kk_ppr, dataset = Twitter from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/kk_ppr file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Twitter/twitter-rv.undirected-reorder firstsource 0 numsources 61578415 blocksize_kb 16384 maxwalklength 16384 walkspersource 1
done

echo "app = kk_ppr, dataset = friendster from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/kk_ppr file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Friendster/out.friendster-undir-reorder firstsource 0 numsources 68349467 blocksize_kb 2048 maxwalklength 16384 walkspersource 1
done

# node2vec
echo "app = node2vec, dataset = Twitter(undirected), L=10 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/node2vec file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Twitter/twitter-rv.undirected-reorder firstsource 12 N 61578415 L 10 blocksize_kb 16384
done

echo "app = node2vec, dataset = friendster(undirected), L=10 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/node2vec file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Friendster/out.friendster-undir-reorder firstsource 0 N 68349467 L 10 blocksize_kb 2048
done

echo "app = node2vec, dataset = Twitter(undirected), L=80 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/node2vec file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Twitter/twitter-rv.undirected-reorder firstsource 12 N 61578415 L 80 blocksize_kb 16384
done

echo "app = node2vec, dataset = friendster(undirected), L=80 from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 5; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/node2vec file ../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Friendster/out.friendster-undir-reorder firstsource 0 N 68349467 L 80 blocksize_kb 2048
done

mv graphwalker_metrics.txt.statistics graphwalker_metrics_fig21.txt
touch graphwalker_metrics.txt.statistics