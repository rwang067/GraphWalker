#!/bin/bash

app=$1
outputfilename=`date '+%Y%m%d%H%M%S'`.csv
echo "APP, PID, time, VIRT, RES, SHR, %cpu, %memory, TIME+" >> ${outputfilename}

while(true)
do
    ret=$(pidof ${app})
    echo ${app}" : "${ret}
    while(true)
    do
        echo ${app} ", "${ret}", "`date '+%Y-%m-%d %H:%M:%S'` ", " `top -n 1 -p ${ret} | grep ${ret} | awk '{print $5","$6","$78","$9","$10","$11}'` >>  ${outputfilename}
        sleep 2
        if [[ `ps -ef |grep -v grep |grep -v monitor |grep ${ret} | wc -l` -eq 0 ]]; then
            break
        fi
    done
    if [[ `ps -ef |grep -v grep |grep -v monitor |grep ${app} | wc -l` -eq 0 ]]; then
        break
    fi
done
