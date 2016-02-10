#!/bin/bash
#
# test.sh
#
# Description: test maze and output results into a log file
# 
# Will conduct a Clean Build and the Unit Testing
#

# 1. Specifiy log files, data directory
logfile="$(date "+%b_%d_%T_%Y").log"
echo "Built by Tom Petty and The HashTables on MacOS 10.10.3" > $logfile
echo "Start testing query" >> $logfile
echo "
">> $logfile

#2. Clean build the query
make clean >> $logfile
make maze_runner >> $logfile
make testing >> $logfile
if [ $? -ne 0 ]; #not zero
then
	echo  "Build failed">> $logfile
	exit 1
fi

echo "Build maze_runner successfully" >> $logfile
echo "
">> $logfile

#3. Unit Testing
echo "Unit testing" >> $logfile
echo "
">> $logfile
echo testing >> $logfile
testing >> $logfile
echo "
">> $logfile

echo "Maze Runs" >> $logfile
echo "
">> $logfile

echo "Invalid Arguments" >> $logfile
echo "
">> $logfile
echo maze_runner 2 1 pierce g >> $logfile
maze_runner 2 1 pierce g >> $logfile
echo "
">> $logfile

echo maze_runner 2 1 pierce1  >> $logfile
maze_runner 2 1 pierce1 >> $logfile
echo "
">> $logfile

echo maze_runner 11 1 pierce  >> $logfile
maze_runner 11 1 pierce >> $logfile
echo "
">> $logfile

echo maze_runner 2 15 pierce  >> $logfile
maze_runner 2 15 pierce >> $logfile
echo "
">> $logfile

echo "Valid Runs: Written to Results Directory" >> $logfile
echo "
">> $logfile

for ((i=0;i<10;i++));
do
	echo maze_runner 2 $i pierce >> $logfile
	maze_runner 2 $i pierce
	echo "
	">> $logfile
	echo maze_runner 5 $i pierce >> $logfile
	maze_runner 5 $i pierce
	echo "
	">> $logfile
	echo maze_runner 8 $i pierce >> $logfile
	maze_runner 8 $i pierce
	echo "
	">> $logfile

done



