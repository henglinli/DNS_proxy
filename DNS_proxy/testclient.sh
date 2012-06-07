#!/bin/sh
LOG=client_test.log
hosts="www.163.com www.google.com.hk www.qq.com www.facebook.com www.uestc.edu.cn youtube.com kernel.org"
echo "client test log begin at" > $LOG
date >> $LOG
for host in $hosts
do
    echo "reslove $host"
    echo "################" >> $LOG
    echo "reslove $host" >> $LOG
    ./client $host >> $LOG
done

echo "end at" >> $LOG
date >> $LOG
