echo "2020.11.23 " >> graphwalker_metrics.txt.statistics 
echo "app = dynamictest, dataset = Friendster, SSD-RAID0" >> graphwalker_metrics.txt.statistics 
echo "Turn on Page Cache, clear pagecache each time" >> graphwalker_metrics.txt.statistics 

################################################################################################
echo "addEdges	compaction	#compaction	compaction_1_loadLog	compaction_2_mallocCSR	compaction_3_loadSubGraph	compaction_4_computeDegree	compaction_5_mallocNewCSR	compaction_6_compBeg	compaction_7_mergeLog2CSR	compaction_8_writeNewCsr	compaction_8_writeNewCsr_splitSubGraph	#compaction_8_writeNewCsr_splitSubGraph	compaction_9_freeCSR	flush	#flush	flush_1_malloc_logs	flush_2_log_classification	flush_3_write_logs	flush_4_free_logs	runtime	nblocks" >> graphwalker_metrics.txt.statistics 
################################################################################################

for(( logsize = 4; logsize <= 64; logsize*=2))
do
    for(( blocksize = logsize/2; blocksize <= logsize*16; blocksize*=2))
    do
        echo "buffer capacity = 262144 edges(2MB), each block's logsize = " $logsize ", blocksize = " $blocksize ", from echo" >> graphwalker_metrics.txt.statistics 
        for(( times = 0; times < 3; times++))
        do
            echo "times = " $times " from echo"
            free -m
            sync; echo 1 > /proc/sys/vm/drop_caches
            free -m
            ./bin/apps/dynamictest file ../../data/raid0_defghij_ssd/Friendster/out.friendster nverts 68349467 logsize $logsize blocksize $blocksize
        done
    done
done