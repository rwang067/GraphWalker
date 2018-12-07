#!/bin/bash

outputfilename=`date '+%Y%m%d%H%M%S'`.csv

echo "time\t VIRT\t RES\t SHR\t %cpu\t %memory\t TIME+\t fd" >> ${outputfilename}

ret=`ps -ef |grep -v grep |grep pagerank |awk '{print $2}'`
#echo ${ret}

while(true)
do
	echo `date '+%Y-%m-%d %H:%M:%S'` "\t" `top -n 1 -p ${ret} | grep ${ret} | awk '{print $5"\t"$6"\t"$7"\t"$9"\t"$10"\t"$11}'` "," `ls -l /proc/${ret}/fd/ | wc -l` >>  ${outputfilename}
sleep 2
if [[ ! -d "/proc/${ret}/fd/" ]]; then
	break
fi
done
