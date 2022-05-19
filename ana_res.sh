#!/bin/bash

declare -i case_num
case_num=`ls -lR ./log_res|grep "^-"|wc -l`/2
declare -i unsolved=0
declare -i solved=0
declare -i unsafe=0
declare -i safe=0
time_cos=0
date=`date +%F`

for file in `ls ./log_res`
do
        if [ ${file##*.} == "res" ]
        then
                size=`ls -l ./log_res/${file} | awk '{print $5}'`
                if [ $size == 0 ] #unsolved, count num
                then
                        unsolved=$unsolved+1
                else #solved, count time cosum
                        str=`grep "Total Time:" ./log_res/${file%.res*}.log`
                        time=${str##*: }
                        time_cos=`echo "$time_cos+$time"|bc`
                        #safe or unsafe
                        str=`sed -n 1p ./log_res/${file%.res*}.res`
                        # if [ $str == "unsafe" ] #unsafe
                        if [ $str == "1" ] #unsafe
                        then
                                echo "${file%.res*}">>unsafe
                                unsafe=$unsafe+1
                        elif [ $str == "0" ] #safe
                        then
                                echo "${file%.res*}">>safe
                                safe=$safe+1
                        fi
                fi
        fi
done
solved=$case_num-$unsolved
echo -e "Total case: \t\t\t${case_num}."
echo -e "unsafe: \t\t\t$unsafe."
echo -e "safe  : \t\t\t$safe."
echo -e "Unsolved in time limit: \t${unsolved}."
echo -e "Total time(solved): \t\t${time_cos} seconds."