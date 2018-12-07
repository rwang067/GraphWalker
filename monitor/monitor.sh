#!/bin/bash

outputfilename=`date '+%Y%m%d%H%M%S'`.csv

<<<<<<< HEAD
echo "time, %cpu, %memory, fd" >> ${outputfilename}
=======
echo "time\t VIRT\t RES\t SHR\t %cpu\t %memory\t TIME+\t fd" >> ${outputfilename}
>>>>>>> be2ee07a6290d4f1757f6940fcf74e4583084412

ret=`ps -ef |grep -v grep |grep pagerank |awk '{print $2}'`
#echo ${ret}

while(true)
do
<<<<<<< HEAD
    echo `date '+%Y-%m-%d %H:%M:%S'` "," `top -n 1 -p ${ret} | grep ${ret} | awk '{print $9","$10}'` "," `ls -l /proc/${ret}/fd/ | wc -l` >>  ${outputfilename}
sleep 5
=======
	echo `date '+%Y-%m-%d %H:%M:%S'` "\t" `top -n 1 -p ${ret} | grep ${ret} | awk '{print $5"\t"$6"\t"$7"\t"$9"\t"$10"\t"$11}'` "," `ls -l /proc/${ret}/fd/ | wc -l` >>  ${outputfilename}
sleep 2
if [[ ! -d "/proc/${ret}/fd/" ]]; then
	break
fi
>>>>>>> be2ee07a6290d4f1757f6940fcf74e4583084412
done
