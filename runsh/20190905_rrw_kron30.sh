

# echo "2019.8.30 " >> graphwalker_metrics.txt.statistics 
# echo "blocksize impact for Graphlet, dataset = Crawl, SSD" >> graphwalker_metrics.txt.statistics 
# echo "Turn on Page Cache, clear pagecache each time" >> graphwalker_metrics.txt.statistics 

# ### 64GB R730, SSD, Crawl
# ################################################################################################
# echo "App = Raw Random Walks" >> graphwalker_metrics.txt.statistics 
# for(( blocksize_kb = 512; blocksize_kb <= 4194304; blocksize_kb*=4))
# do
#     echo "blocksize_kb = " $blocksize_kb >> graphwalker_metrics.txt.statistics 
#     for(( times = 0; times < 3; times++))
#     do
#         echo "times = " $times " from echo"
#         free -m
#         sync; echo 1 > /proc/sys/vm/drop_caches
#         free -m
#         ./bin/apps/rawrandomwalks file ../../raid0_defghij/Crawl/crawl.txt N 3563602789 R 100000 L 10 blocksize_kb $blocksize_kb
#     done
# done

# echo "2019.9.5 " >> graphwalker_metrics.txt.statistics 
# echo "R * L impact for rawrandomwalks, dataset = Kron30, SSD" >> graphwalker_metrics.txt.statistics 
# echo "Turn on Page Cache, clear pagecache each time" >> graphwalker_metrics.txt.statistics 

# ### 64GB R730, SSD, Kron31
# ################################################################################################
# echo "Raw Random walks from echo, L = 10, vary R from 10^3 to 10^10"  >> graphwalker_metrics.txt.statistics
# ## rawrandomwalks
# echo "app = rawrandomwalks, dataset = Crawl from echo" >> graphwalker_metrics.txt.statistics
# for(( R = 1000; R <= 10000000000; R*=10))
# do
#     echo "R = " $R ", fixed L = 10, from echo" >> graphwalker_metrics.txt.statistics 
#     for(( times = 0; times < 2; times++))
#     do
#         echo "times = " $times " from echo"
#         free -m
#         sync; echo 1 > /proc/sys/vm/drop_caches
#         free -m
#         ./bin/apps/rawrandomwalks file ../../raid0_defghij/Kron30/kron30_32-sorted.txt N 1073741823 R $R L 10
#     done
# done

echo "Raw Random walks from echo, R = 100000, vary L from 2^2 to 2^12"  >> graphwalker_metrics.txt.statistics
## rawrandomwalks
echo "app = rawrandomwalks, dataset = Crawl from echo" >> graphwalker_metrics.txt.statistics
for(( L = 32; L <= 4096; L*=2))
do
    echo "L = " $L ", fixed R = 10^5, from echo" >> graphwalker_metrics.txt.statistics
    for(( times = 0; times < 3; times++))
    do
        echo "times = " $times " from echo"
        free -m
        sync; echo 1 > /proc/sys/vm/drop_caches
        free -m
        ./bin/apps/rawrandomwalks file ../../raid0_defghij/Kron30/kron30_32-sorted.txt N 1073741823 R 100000 L $L
    done
done