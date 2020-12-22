################################################### #############################################
echo "2020.12.22 " >> graphwalker_metrics.txt.updatecost 
echo "app = dynamictest, dataset = Friendster, SSD-RAID0" >> graphwalker_metrics.txt.updatecost 
echo "Turn on Page Cache, clear pagecache each time" >> graphwalker_metrics.txt.updatecost 
echo "addEdges	flush	#flush	classifyLog	#classifyLog	writeLog	#writeLog	compaction	#compaction	nblocks	runtime" >> graphwalker_metrics.txt.updatecost 
################################################################################################

################################################################################################
echo "2020.12.22 " >> graphwalker_metrics.txt.querycost 
echo "app = dynamictest, dataset = Friendster, SSD-RAID0" >> graphwalker_metrics.txt.querycost 
echo "Turn on Page Cache, clear pagecache each time" >> graphwalker_metrics.txt.querycost 
echo "test_query	searchNeighbors	#query	InCSR	2_InLogfile	readfile	3_InMembuf	test_traverse" >> graphwalker_metrics.txt.querycost 
################################################################################################


## 3. Two-level blocking
################################################################################################
buffersize=64
nverts_per_grp=16384
logsize=2048
for((blocksize = 4; blocksize <= 32; blocksize*=2))
do
    echo -e "\n buffer capacity = " $buffersize "MB, nverts_per_grp = " $nverts_per_grp ", logsize = " $logsize "KB, blocksize = " $blocksize "MB, from echo" >> graphwalker_metrics.txt.updatecost 
    echo -e "\n buffer capacity = " $buffersize "MB, nverts_per_grp = " $nverts_per_grp ", logsize = " $logsize "KB, blocksize = " $blocksize "MB, from echo" >> graphwalker_metrics.txt.querycost 
    for(( times = 0; times < 3; times++))
    do
        echo "times = " $times " from echo"
        free -m
        sync; echo 1 > /proc/sys/vm/drop_caches
        free -m
        ./bin/apps/dynamictest file ../../data/raid0_defghij_ssd/Friendster/out.friendster N 68349467 buffersize $buffersize  nverts_per_grp $nverts_per_grp logsize $logsize blocksize $blocksize
    done
done
################################################################################################