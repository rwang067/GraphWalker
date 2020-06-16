
echo "app = KK_PPR, dataset = Friendster(undirected) from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/kk_ppr file ../../data/raid0_defghij_ssd/Friendster/out.friendster-undir-reorder numsources 68349467 blocksize_kb 0
done

echo "app = KK_PPR, dataset = Twitter(undirected) from echo" >> graphwalker_metrics.txt.statistics 
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    free -m
    sync; echo 1 > /proc/sys/vm/drop_caches
    free -m
    ./bin/apps/kk_ppr file ../../data/raid0_defghij_ssd/Twitter/twitter-rv.undirected-reorder numsources 61578415 blocksize_kb 0
done



# echo "Seqentially copmuting..." >> graphwalker_metrics.txt.statistics 
# ./bin/apps/msppr file ../../data/raid0_defghij_ssd/Crawl/crawl.txt firstsource 0 nmblocks 1;
# ./bin/apps/simrank file ../../data/raid0_defghij_ssd/Crawl/crawl.txt a 0 b 1 nmblocks 1;
# ./bin/apps/graphlet file ../../data/raid0_defghij_ssd/Crawl/crawl.txt N 3563602789 R 100000 nmblocks 1;
# ./bin/apps/rwdomination file ../../data/raid0_defghij_ssd/Crawl/crawl.txt N 3563602789 nmblocks 1;

# echo "Randomly copmuting..." >> graphwalker_metrics.txt.statistics 
# ./bin/apps/msppr file ../../data/raid0_defghij_ssd/Crawl/crawl.txt firstsource 0 nmblocks 1 &
# ./bin/apps/simrank file ../../data/raid0_defghij_ssd/Crawl/crawl.txt a 0 b 1 nmblocks 1 &
# ./bin/apps/graphlet file ../../data/raid0_defghij_ssd/Crawl/crawl.txt N 3563602789 R 100000 nmblocks 1 & 
# ./bin/apps/rwdomination file ../../data/raid0_defghij_ssd/Crawl/crawl.txt N 3563602789 nmblocks 1

# echo "Randomly copmuting twice..." >> graphwalker_metrics.txt.statistics 
# ./bin/apps/msppr file ../../data/raid0_defghij_ssd/Crawl/crawl.txt firstsource 0 nmblocks 1 &
# ./bin/apps/simrank file ../../data/raid0_defghij_ssd/Crawl/crawl.txt a 0 b 1 nmblocks 1 &
# ./bin/apps/graphlet file ../../data/raid0_defghij_ssd/Crawl/crawl.txt N 3563602789 R 100000 nmblocks 1 & 
# ./bin/apps/rwdomination file ../../data/raid0_defghij_ssd/Crawl/crawl.txt N 3563602789 nmblocks 1 &
# ./bin/apps/msppr file ../../data/raid0_defghij_ssd/Crawl/crawl.txt firstsource 0 nmblocks 1 &
# ./bin/apps/simrank file ../../data/raid0_defghij_ssd/Crawl/crawl.txt a 0 b 1 nmblocks 1 &
# ./bin/apps/graphlet file ../../data/raid0_defghij_ssd/Crawl/crawl.txt N 3563602789 R 100000 nmblocks 1 & 
# ./bin/apps/rwdomination file ../../data/raid0_defghij_ssd/Crawl/crawl.txt N 3563602789 nmblocks 1 &
    

# ### 64GB R730, SSD, Crawl
# ################################################################################################
# echo "app = single-source RW, dataset = Crawl from echo" >> graphwalker_metrics.txt.statistics 
# for(( R = 1000; R <= 1000000000; R*=10))
# do
#     echo "R = " $R ",  R*10 walks for the soource from echo" >> graphwalker_metrics.txt.statistics 
#     for(( times = 0; times < 5; times++))
#     do
#         echo "times = " $times " from echo"
#         free -m
#         sync; echo 1 > /proc/sys/vm/drop_caches
#         free -m
#         ./bin/apps/msppr file ../../raid0_defghij/Crawl/crawl.txt firstsource 0 numsources 1 walkspersource $R
#     done
# done
