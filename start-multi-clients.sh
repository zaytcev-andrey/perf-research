#!/bin/bash

CLIENTS_COUNT_DEFAULT=20
CLIENTS_COUNT=$CLIENTS_COUNT_DEFAULT

FILES_COUNT_DEFAULT=1000
FILES_COUNT=$FILES_COUNT_DEFAULT

if [ "$1" = "help" ]
then
	echo "usage: start-milti-clients.sh <clients count = $CLIENTS_COUNT_DEFAULT as defualt> <files count = $FILES_COUNT_DEFAULT as default>"
	exit 0
fi

if [ "$1" != "" ]
then
	CLIENTS_COUNT=$1
fi

if [ "$2" != "" ]
then
	FILES_COUNT=$2
	sleep	3
fi

echo "will start $CLIENTS_COUNT clients with $FILES_COUNT files to receive..."
sleep	3

for (( CLIENT=1; CLIENT <= $CLIENTS_COUNT; CLIENT++ ))
do
	echo "starting client $CLIENT"
	./client/perf-client.exe -f $FILES_COUNT & #> /dev/null 2>&1 &
done
