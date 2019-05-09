# !/bin/bash

# 2018.12.27
# Friendster MSPPR
    echo "2018.12.27 observe the effect of shardsize for MSPPR in PC(dell)" >> graphchi_metrics.txt.statistics 
    echo "app = MSPPR, dataset = Friendster" >> graphchi_metrics.txt.statistics 

# numsources = 1000, walkspersource = 2000, firstsource = 0
    echo "numsources = 1000, walkspersource = 2000, firstsource = 12" >> graphchi_metrics.txt.statistics 
    for(( shardsize = 1048576; shardsize <= 1048576; shardsize*=2))
    do
        echo "shardsize = " $shardsize >> graphchi_metrics.txt.statistics 
        for(( numsources = 1; numsources <= 10000; numsources*=10))
        do
            echo "shardsize = " $shardsize ", numsources = " $numsources >> graphchi_metrics.txt.statistics 
            for(( times = 0; times < 5; times++))
            do
                echo "times = " $times " from echo"
                ./bin/apps/msppr file  ../DataSet/Friendster/out.friendster-reorder firstsource 12 numsources $numsources shardsize $shardsize
            done
        done
	done

# # numsources = 1, walkspersource = 2000, firstsource = 0
#     echo "numsources = 1(SSPPR), walkspersource = 2000, firstsource = 0" >> graphchi_metrics.txt.statistics 
#     for(( shardsize = 32; shardsize <= 1048576; shardsize*=2))
#     do
#         echo "shardsize = " $shardsize >> graphchi_metrics.txt.statistics 
# 	    for(( times = 0; times < 5; times++))
# 	    do
# 	        echo "times = " $times " from echo"
# 	        ./bin/apps/msppr file  ../DataSet/Friendster/out.friendster-reorder firstsource 12 numsources 1 shardsize $shardsize
#             done
# 	done


# # numsources = 10000, walkspersource = 2000, firstsource = 0
#     echo "numsources = 10000, walkspersource = 2000, firstsource = 0" >> graphchi_metrics.txt.statistics 
#     for(( shardsize = 32; shardsize <= 1048576; shardsize*=2))
#     do
#         echo "shardsize = " $shardsize >> graphchi_metrics.txt.statistics 
# 	    for(( times = 0; times < 5; times++))
# 	    do
# 	        echo "times = " $times " from echo"
# 	        ./bin/apps/msppr file  ../DataSet/Friendster/out.friendster-reorder firstsource 12 numsources 10000 shardsize $shardsize
#             done
# 	done

# # numsources = 10, walkspersource = 2000, firstsource = 0
#     echo "numsources = 10, walkspersource = 2000, firstsource = 0" >> graphchi_metrics.txt.statistics 
#     for(( shardsize = 32; shardsize <= 1048576; shardsize*=2))
#     do
#         echo "shardsize = " $shardsize >> graphchi_metrics.txt.statistics 
# 	    for(( times = 0; times < 5; times++))
# 	    do
# 	        echo "times = " $times " from echo"
# 	        ./bin/apps/msppr file  ../DataSet/Friendster/out.friendster-reorder firstsource 12 numsources 10 shardsize $shardsize
#             done
# 	done

# # numsources = 100, walkspersource = 2000, firstsource = 0
#     echo "numsources = 100, walkspersource = 2000, firstsource = 0" >> graphchi_metrics.txt.statistics 
#     for(( shardsize = 32; shardsize <= 1048576; shardsize*=2))
#     do
#         echo "shardsize = " $shardsize >> graphchi_metrics.txt.statistics 
# 	    for(( times = 0; times < 5; times++))
# 	    do
# 	        echo "times = " $times " from echo"
# 	        ./bin/apps/msppr file  ../DataSet/Friendster/out.friendster-reorder firstsource 12 numsources 100 shardsize $shardsize
#             done
# 	done
