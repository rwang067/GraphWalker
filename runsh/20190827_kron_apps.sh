
# echo "2019.8.26 " >> graphwalker_metrics.txt.statistics 
# echo "random walks applications performance comparison (clear pagecache each time) in 64GB R730, dataset = Kron30, SSD" >> graphwalker_metrics.txt.statistics 
# echo "Turn on Page Cache, clear pagecache each time" >> graphwalker_metrics.txt.statistics 

# ### 64GB R730, SSD, Kron30
# ################################################################################################
# echo "App = PPR" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/msppr file ../../raid0_defghij/Kron30/kron30_32-sorted.txt firstsource 0
# done

# ### 64GB R730, SSD, Kron30
# ################################################################################################
# echo "App = SimRank" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/simrank file ../../raid0_defghij/Kron30/kron30_32-sorted.txt a 0 b 7
# done

# ### 64GB R730, SSD, Kron30
# ################################################################################################
# echo "App = Graphlet" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/graphlet file ../../raid0_defghij/Kron30/kron30_32-sorted.txt N 1073741823‬ R 100000
# done

## 64GB R730, SSD, Kron30
###############################################################################################
# echo "App = RandomWalkDomination" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/rwdomination file ../../raid0_defghij/Kron30/kron30_32-sorted.txt N 10737418243
# done


# echo "2019.8.26 " >> graphwalker_metrics.txt.statistics 
# echo "random walks applications performance comparison (clear pagecache each time) in 64GB R730, dataset = Kron31, SSD" >> graphwalker_metrics.txt.statistics 
# echo "Turn on Page Cache, clear pagecache each time" >> graphwalker_metrics.txt.statistics 

# ### 64GB R730, SSD, Kron31
# ################################################################################################
# echo "App = PPR" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/msppr file ../../raid0_defghij/Kron31/kron31_32-sorted.txt firstsource 0
# done

# ### 64GB R730, SSD, Kron31
# ################################################################################################
# echo "App = SimRank" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/simrank file ../../raid0_defghij/Kron31/kron31_32-sorted.txt a 0 b 6
# done

# ### 64GB R730, SSD, Kron31
# ################################################################################################
# echo "App = Graphlet" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/graphlet file ../../raid0_defghij/Kron31/kron31_32-sorted.txt N 2147483648 R 100000
# done

# ## 64GB R730, SSD, Kron31
# ###############################################################################################
# echo "App = RandomWalkDomination" >> graphwalker_metrics.txt.statistics 
# for(( times = 0; times < 5; times++))
# do
#     echo "times = " $times " from echo"
#     free -m
#     sync; echo 1 > /proc/sys/vm/drop_caches
#     free -m
#     ./bin/apps/rwdomination file ../../raid0_defghij/Kron31/kron31_32-sorted.txt N 2147483648
# done

echo "2019.8.30 " >> graphwalker_metrics.txt.statistics 
echo "blocksize impact for Graphlet, dataset = Crawl, SSD" >> graphwalker_metrics.txt.statistics 
echo "Turn on Page Cache, clear pagecache each time" >> graphwalker_metrics.txt.statistics 

### 64GB R730, SSD, Crawl
################################################################################################
echo "App = Raw Random Walks" >> graphwalker_metrics.txt.statistics 
for(( blocksize_kb = 512; blocksize_kb <= 4194304; blocksize_kb*=4))
do
    echo "blocksize_kb = " $blocksize_kb >> graphwalker_metrics.txt.statistics 
    for(( times = 0; times < 3; times++))
    do
        echo "times = " $times " from echo"
        free -m
        sync; echo 1 > /proc/sys/vm/drop_caches
        free -m
        ./bin/apps/rawrandomwalks file ../../raid0_defghij/Crawl/crawl.txt N 3563602789 R 100000 L 10 blocksize_kb $blocksize_kb
    done
done

# echo "2019.8.27 " >> graphwalker_metrics.txt.statistics 
# echo "R * L impact for rawrandomwalks, dataset = Crawl, SSD" >> graphwalker_metrics.txt.statistics 
# echo "Turn on Page Cache, clear pagecache each time" >> graphwalker_metrics.txt.statistics 

# ### 64GB R730, SSD, Kron31
# ################################################################################################
# echo "Raw Random walks from echo, L = 10, vary R from 10^3 to 10^10"  >> grafsoft.statistics 
# ## rawrandomwalks
# echo "app = rawrandomwalks, dataset = Crawl from echo" >> grafsoft.statistics 
# for(( R = 1000; R <= 10000000000; R*=10))
# do
#     echo "R = " $R ", fixed L = 10, from echo" >> graphwalker_metrics.txt.statistics 
#     for(( times = 0; times < 2; times++))
#     do
#         echo "times = " $times " from echo"
#         free -m
#         sync; echo 1 > /proc/sys/vm/drop_caches
#         free -m
#         ./bin/apps/rawrandomwalks file ../../raid0_defghij/Crawl/crawl.txt N 3563602789 R $R L 10
#     done
# done

# echo "Raw Random walks from echo, R = 100000, vary L from 2^2 to 2^12"  >> grafsoft.statistics 
# ## rawrandomwalks
# echo "app = rawrandomwalks, dataset = Crawl from echo" >> grafsoft.statistics 
# for(( L = 4; L <= 4096; L*=2))
# do
#     echo "L = " $L ", fixed R = 10^5, from echo" >> grafsoft.statistics 
#     for(( times = 0; times < 5; times++))
#     do
#         echo "times = " $times " from echo"
#         free -m
#         sync; echo 1 > /proc/sys/vm/drop_caches
#         free -m
#         ./bin/apps/rawrandomwalks file ../../raid0_defghij/Crawl/crawl.txt N 3563602789 R 100000 L $L
#     done
# done