
echo "2019.9.15 compare with Graphene/GraFSoft by single-source RW in Friendster in SSD 64GB R730" >> graphwalker_metrics.txt.statistics 
echo "MSPPR from echo, L = 10, R vary from 10^4 to 10^7" >> graphwalker_metrics.txt.statistics 

### 64GB R730, SSD, Friendster
################################################################################################
echo "app = single-source RW, dataset = Friendster from echo" >> graphwalker_metrics.txt.statistics 
for(( R = 1000; R <= 1000000000; R*=10))
do
    echo "R = " $R ",  R*10 walks for the soource from echo" >> graphwalker_metrics.txt.statistics 
    for(( times = 0; times < 5; times++))
    do
        echo "times = " $times " from echo"
        free -m
        sync; echo 1 > /proc/sys/vm/drop_caches
        free -m
        ./bin/apps/msppr file ../../raid0_defghij/Friendster/out.friendster-reorder firstsource 12 numsources 1 walkspersource $R
    
    done
done


echo "2019.9.15 compare with Graphene/GraFSoft by single-source RW in Crawl in SSD 64GB R730" >> graphwalker_metrics.txt.statistics 
echo "MSPPR from echo, 2000*10 walks for each soource, numsources from 10^0 to 10^7" >> graphwalker_metrics.txt.statistics 

### 64GB R730, SSD, Crawl
################################################################################################
echo "app = single-source RW, dataset = Crawl from echo" >> graphwalker_metrics.txt.statistics 
for(( R = 1000; R <= 1000000000; R*=10))
do
    echo "R = " $R ",  R*10 walks for the soource from echo" >> graphwalker_metrics.txt.statistics 
    for(( times = 0; times < 5; times++))
    do
        echo "times = " $times " from echo"
        free -m
        sync; echo 1 > /proc/sys/vm/drop_caches
        free -m
        ./bin/apps/msppr file ../../raid0_defghij/Crawl/crawl.txt firstsource 0 numsources 1 walkspersource $R
    done
done
