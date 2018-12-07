#!/bin/bash

outputfilename=`date '+%Y%m%d%H%M%S'`.csv

echo "time, %cpu, %memory, fd" >> ${outputfilename}

ret=`ps -ef |grep -v grep |grep pagerank |awk '{print $2}'`
#echo ${ret}

while(true)
do
    echo `date '+%Y-%m-%d %H:%M:%S'` "," `top -n 1 -p ${ret} | grep ${ret} | awk '{print $9","$10}'` "," `ls -l /proc/${ret}/fd/ | wc -l` >>  ${outputfilename}
sleep 5
done
