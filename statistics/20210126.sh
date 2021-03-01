################################################### #############################################
echo "2020.12.29 uniform #V blocking logcap = 16k" >> graphwalker_metrics.txt.updatecost 
echo "app = dynamictest, dataset = Friendster, SSD-RAID0" >> graphwalker_metrics.txt.updatecost 
echo "Turn on Page Cache, clear pagecache each time" >> graphwalker_metrics.txt.updatecost 
echo "addEdges	flush	#flush	classifyLog	#classifyLog	writeLog	#writeLog	compaction	#compaction	nblocks	runtime" >> graphwalker_metrics.txt.updatecost 
################################################################################################

################################################################################################
echo "2020.12.29 uniform #V blocking logcap = 16k" >> graphwalker_metrics.txt.querycost 
echo "app = dynamictest, dataset = Friendster, SSD-RAID0" >> graphwalker_metrics.txt.querycost 
echo "Turn on Page Cache, clear pagecache each time" >> graphwalker_metrics.txt.querycost 
echo "test_query	searchNeighbors	#query	InCSR	2_InLogfile	readfile	3_InMembuf	test_traverse" >> graphwalker_metrics.txt.querycost 
################################################################################################

################################################################################################
echo -e "\n Impact of nverts_per_grp, from echo" >> graphwalker_metrics.txt.updatecost 
echo -e "\n Impact of nverts_per_grp, from echo" >> graphwalker_metrics.txt.querycost
################################################################################################
buffersize=2
nverts_per_grp=524288
logsize=2048
for((logsize = 64; logsize <= 2048; logsize*=2))
do
    echo -e "\n buffer capacity = " $buffersize "MB, logsize = " $logsize "KB, nverts_per_grp = " $nverts_per_grp ", from echo" >> graphwalker_metrics.txt.updatecost 
    echo -e "\n buffer capacity = " $buffersize "MB, logsize = " $logsize "KB, nverts_per_grp = " $nverts_per_grp ", from echo" >> graphwalker_metrics.txt.querycost 
    for(( times = 0; times < 3; times++))
    do
        echo "times = " $times " from echo"
        free -m
        sync; echo 1 > /proc/sys/vm/drop_caches
        free -m
        ./bin/apps/dynamictest file ../../data/raid0_defghij_ssd/Friendster/out.friendster buffersize $buffersize logsize $logsize nverts_per_grp $nverts_per_grp 
    done
done
################################################################################################


# ################################################################################################
# echo -e "\n Impact of nverts_per_grp, from echo" >> graphwalker_metrics.txt.updatecost 
# echo -e "\n Impact of nverts_per_grp, from echo" >> graphwalker_metrics.txt.querycost
# ################################################################################################
# buffersize=2
# logsize=2048
# for((nverts_per_grp = 4096; nverts_per_grp <= 1048576; nverts_per_grp*=2))
# do
#     echo -e "\n buffer capacity = " $buffersize "MB, logsize = " $logsize "KB, nverts_per_grp = " $nverts_per_grp ", from echo" >> graphwalker_metrics.txt.updatecost 
#     echo -e "\n buffer capacity = " $buffersize "MB, logsize = " $logsize "KB, nverts_per_grp = " $nverts_per_grp ", from echo" >> graphwalker_metrics.txt.querycost 
#     for(( times = 0; times < 3; times++))
#     do
#         echo "times = " $times " from echo"
#         free -m
#         sync; echo 1 > /proc/sys/vm/drop_caches
#         free -m
#         ./bin/apps/dynamictest file ../../data/raid0_defghij_ssd/Friendster/out.friendster buffersize $buffersize logsize $logsize nverts_per_grp $nverts_per_grp 
#     done
# done
# ################################################################################################

# ################################################################################################
# echo -e "\n Impact of blocksize, from echo" >> graphwalker_metrics.txt.updatecost 
# echo -e "\n Impact of blocksize, from echo" >> graphwalker_metrics.txt.querycost
# ################################################################################################
# buffersize=2
# logsize=2048
# for((nverts_per_grp = 65536; nverts_per_grp <= 524288; nverts_per_grp*=2))
# do
#     for((blocksize = 512; blocksize >= 4; blocksize/=2))
#     do
#         echo -e "\n buffer capacity = " $buffersize "MB, logsize = " $logsize "KB, nverts_per_grp = " $nverts_per_grp ", blocksize = " $blocksize ", from echo" >> graphwalker_metrics.txt.updatecost 
#         echo -e "\n buffer capacity = " $buffersize "MB, logsize = " $logsize "KB, nverts_per_grp = " $nverts_per_grp ", blocksize = " $blocksize ", from echo" >> graphwalker_metrics.txt.querycost 
#         for(( times = 0; times < 3; times++))
#         do
#             echo "times = " $times " from echo"
#             free -m
#             sync; echo 1 > /proc/sys/vm/drop_caches
#             free -m
#             ./bin/apps/dynamictest file ../../data/raid0_defghij_ssd/Friendster/out.friendster buffersize $buffersize  logsize $logsize nverts_per_grp $nverts_per_grp blocksize $blocksize
#         done
#     done
# done
# ################################################################################################