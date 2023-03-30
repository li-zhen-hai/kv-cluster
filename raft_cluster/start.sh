#! /bin/bash

while true 
do
	monitor=`ps -ef | grep raft_server_1 | grep -v grep | wc -l ` 
	if [ $monitor -eq 0 ] 
	then
		./../bin/test/raft_cluster/raft_server_1 > log1.log &
	fi

    monitor=`ps -ef | grep raft_server_2 | grep -v grep | wc -l ` 
	if [ $monitor -eq 0 ] 
	then
		./../bin/test/raft_cluster/raft_server_2 > log2.log &
	fi

    monitor=`ps -ef | grep raft_server_3 | grep -v grep | wc -l ` 
	if [ $monitor -eq 0 ] 
	then
		./../bin/test/raft_cluster/raft_server_3 > log3.log &
	fi

	sleep 30
done
