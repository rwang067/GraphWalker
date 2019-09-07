
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

echo "2019.9.4 compare with GraFSoft by msppr in Crawl in SSD 64GB R730" >> graphwalker_metrics.txt.statistics 
echo "MSPPR from echo, 2000*10 walks for each soource, numsources from 10^0 to 10^7" >> graphwalker_metrics.txt.statistics 

### 64GB R730, SSD, Crawl
################################################################################################
echo "app = MSPPR, dataset = Crawl from echo" >> graphwalker_metrics.txt.statistics 
for(( numsources = 1; numsources <= 10000000; numsources*=10))
do
    echo "numsources = " $numsources ", fixed 2000*10 walks for each soource from echo" >> graphwalker_metrics.txt.statistics 
    for(( times = 0; times < 5; times++))
    do
        echo "times = " $times " from echo"
        free -m
        sync; echo 1 > /proc/sys/vm/drop_caches
        free -m
        ./bin/apps/msppr file ../../raid0_defghij/Crawl/crawl.txt firstsource 0 numsources $numsources
    done
done
