#!/bin/bash

IP_DEFAULT=127.0.0.1
IP=$IP_DEFAULT

PORT_DEFAULT=12345
PORT=$PORT_DEFAULT

CLIENTS_COUNT_DEFAULT=20
CLIENTS_COUNT=$CLIENTS_COUNT_DEFAULT

FILES_COUNT_DEFAULT=1000
FILES_COUNT=$FILES_COUNT_DEFAULT

if [ "$1" = "help" ]
then
	echo "usage: start-milti-clients.sh" \
	" -h <remote host = $IP_DEFAULT as default>" \
	" -p <remote port = $PORT_DEFAULT as default>" \
	" -n <clients count = $CLIENTS_COUNT_DEFAULT as defualt>" \
	" -f <files count = $FILES_COUNT_DEFAULT as default>"
	exit 0
fi

while getopts :h:p:n:f: ARG
do
	case $ARG in
		h) IP=$OPTARG;;
		p) PORT=$OPTARG;;
		n) CLIENTS_COUNT=$OPTARG;;
		f) FILES_COUNT=$OPTARG;;
	esac
done

echo "will start $CLIENTS_COUNT clients with $FILES_COUNT files to receive and connected to $IP:$PORT ..."
sleep	3

for (( CLIENT=1; CLIENT <= $CLIENTS_COUNT; CLIENT++ ))
do
	echo "starting client $CLIENT"
	./client/perf-client.exe -h $IP -p $PORT -f $FILES_COUNT & #> /dev/null 2>&1 &
done
