
# echo "2019.9.7 compare with GraFSoft by msppr in Friendster in SSD 64GB R730" >> graphwalker_metrics.txt.statistics 
# echo "MSPPR from echo, 2000*10 walks for each soource, numsources from 10^0 to 10^7" >> graphwalker_metrics.txt.statistics 

# ### 64GB R730, SSD, Friendster
# ################################################################################################
# echo "app = MSPPR, dataset = Friendster from echo" >> graphwalker_metrics.txt.statistics 
# for(( numsources = 1; numsources <= 10000000; numsources*=10))
# do
#     echo "numsources = " $numsources ", fixed 2000*10 walks for each soource from echo" >> graphwalker_metrics.txt.statistics 
#     for(( times = 0; times < 5; times++))
#     do
#         echo "times = " $times " from echo"
#         free -m
#         sync; echo 1 > /proc/sys/vm/drop_caches
#         free -m
#         ./bin/apps/msppr file ../../raid0_defghij/Friendster/out.friendster-reorder firstsource 12 numsources $numsources
#     done
# done


# echo "2019.9.7 compare with GraFSoft by msppr in Crawl in SSD 64GB R730" >> graphwalker_metrics.txt.statistics 
# echo "MSPPR from echo, 2000*10 walks for each soource, numsources from 10^0 to 10^7" >> graphwalker_metrics.txt.statistics 

# ### 64GB R730, SSD, Crawl
# ################################################################################################
# echo "app = MSPPR, dataset = Crawl from echo" >> graphwalker_metrics.txt.statistics 
# for(( numsources = 1; numsources <= 10000000; numsources*=10))
# do
#     echo "numsources = " $numsources ", fixed 2000*10 walks for each soource from echo" >> graphwalker_metrics.txt.statistics 
#     for(( times = 0; times < 5; times++))
#     do
#         echo "times = " $times " from echo"
#         free -m
#         sync; echo 1 > /proc/sys/vm/drop_caches
#         free -m
#         ./bin/apps/msppr file ../../raid0_defghij/Crawl/crawl.txt firstsource 0 numsources $numsources
#     done
# done

echo "2019.9.5 " >> graphwalker_metrics.txt.statistics 
echo "R * L impact for rawrandomwalks, dataset = Kron30, SSD" >> graphwalker_metrics.txt.statistics 
echo "Turn on Page Cache, clear pagecache each time" >> graphwalker_metrics.txt.statistics 

### 64GB R730, SSD, Kron31
################################################################################################
echo "Raw Random walks from echo, L = 10, vary R from 10^3 to 10^10"  >> graphwalker_metrics.txt.statistics
## rawrandomwalks
echo "app = rawrandomwalks, dataset = Crawl from echo" >> graphwalker_metrics.txt.statistics
for(( R = 1000; R <= 10000000000; R*=10))
do
    echo "R = " $R ", fixed L = 10, from echo" >> graphwalker_metrics.txt.statistics 
    for(( times = 0; times < 2; times++))
    do
        echo "times = " $times " from echo"
        free -m
        sync; echo 1 > /proc/sys/vm/drop_caches
        free -m
        ./bin/apps/rawrandomwalks file ../../raid0_defghij/Kron30/kron30_32-sorted.txt N 1073741823 R $R L 10
    done
done

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