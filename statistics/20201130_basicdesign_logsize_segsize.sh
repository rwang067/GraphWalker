echo "2020.11.27 " >> graphwalker_metrics.txt.statistics 
echo "app = dynamictest, dataset = Friendster, SSD-RAID0" >> graphwalker_metrics.txt.statistics 
echo "Turn on Page Cache, clear pagecache each time" >> graphwalker_metrics.txt.statistics 

################################################################################################
echo "addEdges	compaction	#compaction	1_loadLog	2_computeDegree	3_loadSubGraph	4_mallocNewCSR	5_compBeg	6_copyCSR	7_mergeLog2CSR	8_splitSubGraph	#splitSubGraph	8_writeSubGraph	9_free	flush	#flush	1_malloc	2_log_classification	3_write_logs	4_free	runtime" >> graphwalker_metrics.txt.statistics 
################################################################################################

for(( logsize = 4; logsize <= 64; logsize*=2))
do
    for(( segsize = logsize/2; segsize <= logsize*16; segsize*=2))
    do
        echo "buffer capacity = 262144 edges(2MB), each block's logsize = " $logsize ", segsize = " $segsize ", from echo" >> graphwalker_metrics.txt.statistics 
        for(( times = 0; times < 3; times++))
        do
            echo "times = " $times " from echo"
            free -m
            sync; echo 1 > /proc/sys/vm/drop_caches
            free -m
            ./bin/apps/dynamictest file ../../data/raid0_defghij_ssd/Friendster/out.friendster N 68349467 logsize $logsize segsize $segsize
        done
    done
done