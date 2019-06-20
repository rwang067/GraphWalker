echo "2019.6.20 " >> graphwalker_metrics.txt.statistics
echo "observe the impact of block size (clear pagecache each time) in 64GB R730, app = msppr, dataset = Crawl" >> graphwalker_metrics.txt.statistics

### 64GB R730, SSD, Crawl
################################################################################################
for(( numsources = 100; numsources <= 100; numsources*=10))
do
    echo "numsources = " $numsources >> graphwalker_metrics.txt.statistics
    echo "Turn on Page Cache, clear pagecache each time, nmblocks = 1" >> graphwalker_metrics.txt.statistics
    for(( blocksize_kb = 4096; blocksize_kb >= 512; blocksize_kb/=2))
    do
        echo "Turn on Page Cache, blocksize_kb = " $blocksize_kb >> graphwalker_metrics.txt.statistics
        for(( times = 0; times < 3; times++))
        do
            echo "times = " $times " from echo"
            free -m
            sync; echo 1 > /proc/sys/vm/drop_caches
            free -m
            ./bin/apps/msppr file ../../raid0_mnop/Crawl/crawl.txt firstsource 0 numsources $numsources nmblocks 1 blocksize_kb $blocksize_kb
        done
    done
done

for(( numsources = 10000; numsources <= 10000; numsources*=10))
do
    echo "numsources = " $numsources >> graphwalker_metrics.txt.statistics
    echo "Turn on Page Cache, clear pagecache each time, nmblocks = 1" >> graphwalker_metrics.txt.statistics
    for(( blocksize_kb = 8192; blocksize_kb >= 1024; blocksize_kb/=2))
    do
        echo "Turn on Page Cache, blocksize_kb = " $blocksize_kb >> graphwalker_metrics.txt.statistics
        for(( times = 0; times < 3; times++))
        do
            echo "times = " $times " from echo"
            free -m
            sync; echo 1 > /proc/sys/vm/drop_caches
            free -m
            ./bin/apps/msppr file ../../raid0_mnop/Crawl/crawl.txt firstsource 0 numsources $numsources nmblocks 1 blocksize_kb $blocksize_kb
        done
    done
done