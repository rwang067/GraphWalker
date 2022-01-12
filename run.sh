echo -e "Runtime \t chooseBlock \t findSubGraph \t getCurrentWalks \t writeWalks2Disk \t exec_update \t updateWalkNum \t fine_grained_updates \t #blocks" >> graphwalker_metrics.txt.statistics

for i in 18 16 20
do
    echo "run_fig$i.sh"
    bash ./runsh/run_fig$i.sh
    sleep 5
done

