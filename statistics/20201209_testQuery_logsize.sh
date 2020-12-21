echo "2020.12.8 " >> graphwalker_metrics.txt.statistics 
echo "app = dynamictest, dataset = Friendster, SSD-RAID0" >> graphwalker_metrics.txt.statistics 
echo "Turn on Page Cache, clear pagecache each time" >> graphwalker_metrics.txt.statistics 

################################################################################################
echo "addEdges	compaction	#compaction	1_loadLog	2_computeDegree	3_loadSubGraph	4_mallocNewCSR	5_compBeg	6_copyCSR	7_mergeLog2CSR	8_splitSubGraph	#splitSubGraph	8_writeSubGraph	9_free	flush	#flush	1_malloc	2_log_classification	3_write_logs	4_free	runtime" >> graphwalker_metrics.txt.statistics 
echo "test_query	test_searchNeighbors	test_searchNeighbors_1_InSegmentCSR	test_searchNeighbors_2_InLogfile	test_searchNeighbors_3_InMembuf" >> graphwalker_metrics.txt.statistics 
################################################################################################

## 3. Two-level blocking -- test query cost
#####################################
echo "3. Two-level blocking" >> graphwalker_metrics.txt.statistics 
nverts_per_blk=1048576
segsize=64
for((logsize = 2; logsize <= 256; logsize*=2))
do
    echo "buffer capacity = 2MB, each block's #vertices = " $nverts_per_blk ", segsize = " $segsize ", logsize = " $logsize ", from echo" >> graphwalker_metrics.txt.statistics 
    for(( times = 0; times < 3; times++))
    do
        echo "times = " $times " from echo"
        free -m
        sync; echo 1 > /proc/sys/vm/drop_caches
        free -m
        ./bin/apps/dynamictest file ../../data/raid0_defghij_ssd/Friendster/out.friendster N 68349467 nverts_per_blk $nverts_per_blk segsize $segsize logsize $logsize 
    done
done