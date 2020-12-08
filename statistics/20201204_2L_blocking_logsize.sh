echo "2020.12.3 " >> graphwalker_metrics.txt.statistics 
echo "app = dynamictest, dataset = Friendster, SSD-RAID0" >> graphwalker_metrics.txt.statistics 
echo "Turn on Page Cache, clear pagecache each time" >> graphwalker_metrics.txt.statistics 

################################################################################################
echo "addEdges	compaction	#compaction	1_loadLog	2_computeDegree	3_loadSubGraph	4_mallocNewCSR	5_compBeg	6_copyCSR	7_mergeLog2CSR	8_splitSubGraph	#splitSubGraph	8_writeSubGraph	9_free	flush	#flush	1_malloc	2_log_classification	3_write_logs	4_free	runtime" >> graphwalker_metrics.txt.statistics 
################################################################################################

# ## 1. uniform #vertices in each block
# #####################################
# echo "1. uniform #vertices in each block" >> graphwalker_metrics.txt.statistics 
# logsize=4
# segsize=65536
# for((nverts_per_blk = 32768; nverts_per_blk <= 2097152; nverts_per_blk*=2))
# do
#     echo "buffer capacity = 2MB, each block's #vertices = " $nverts_per_blk ", logsize = " $logsize ", segsize = " $segsize ", from echo" >> graphwalker_metrics.txt.statistics 
#     for(( times = 0; times < 3; times++))
#     do
#         echo "times = " $times " from echo"
#         free -m
#         sync; echo 1 > /proc/sys/vm/drop_caches
#         free -m
#         ./bin/apps/dynamictest file ../../data/raid0_defghij_ssd/Friendster/out.friendster N 68349467 nverts_per_blk $nverts_per_blk logsize $logsize segsize $segsize
#     done
# done

# ## 2. uniform #edges in each block
# #####################################
# echo "2. uniform #edges in each block" >> graphwalker_metrics.txt.statistics 
# nverts_per_blk=134217728
# logsize=4
# for((segsize = 8; segsize <= 128; segsize*=2))
# do
#     echo "buffer capacity = 2MB, each block's #vertices = " $nverts_per_blk ", logsize = " $logsize ", segsize = " $segsize ", from echo" >> graphwalker_metrics.txt.statistics 
#     for(( times = 0; times < 3; times++))
#     do
#         echo "times = " $times " from echo"
#         free -m
#         sync; echo 1 > /proc/sys/vm/drop_caches
#         free -m
#         ./bin/apps/dynamictest file ../../data/raid0_defghij_ssd/Friendster/out.friendster N 68349467 nverts_per_blk $nverts_per_blk logsize $logsize segsize $segsize
#     done
# done

## 3. Two-level blocking
#####################################
echo "3. Two-level blocking" >> graphwalker_metrics.txt.statistics 
logsize=1
for((nverts_per_blk = 32768; nverts_per_blk <= 2097152; nverts_per_blk*=2))
do
    let logsize*=2
    for((segsize = 32; segsize <= 256; segsize*=2))
    do
        echo "buffer capacity = 2MB, each block's #vertices = " $nverts_per_blk ", logsize = " $logsize ", segsize = " $segsize ", from echo" >> graphwalker_metrics.txt.statistics 
        for(( times = 0; times < 3; times++))
        do
            echo "times = " $times " from echo"
            free -m
            sync; echo 1 > /proc/sys/vm/drop_caches
            free -m
            ./bin/apps/dynamictest file ../../data/raid0_defghij_ssd/Friendster/out.friendster N 68349467 nverts_per_blk $nverts_per_blk logsize $logsize segsize $segsize
        done
    done
done