#!/bin/bash

# Argparse
PARAMS=""
while (( "$#" )); do
    case "$1" in
        -PASS|--PASSWORD)
            PASSWORD=$2
            export SSHPASS=$PASSWORD
            shift
            ;;
        -u|--user)
            USER=$2
            shift
            ;;
        -p|--ip)
            IP=$2
            shift
            ;;
        -P|--ip)
            PORT=$2
            shift
            ;;
        -d|--upload-dicrectory)
            DIR=$2
            shift
            ;;
        -t|--time-lapse)
            TIME=$2
            shift
            ;;
        -*|--*=) # unsupported flags
            echo "Error: Unsupported flag $1" >&2
            exit 1
            ;;
        *) #reserve positional arguments
            PARAMS="$PARAMS $1"
            shift
            ;;
    esac
done
eval set -- "$PARAMS"

# grep which node this is
RESULT=$(echo $DIR |grep -o "NODE[0-9][0-9]")
printf "$(date '+%D %T')\n$RESULT starts backing up..."| telegram-send -g --stdin
mkdir -p /home/data/

while true
do
    echo "$(date '+%D %T'): backing up"
    # Start rsync
    OUTPUT=$(sshpass -e rsync -az --out-format="%t %n %'''b" --remove-source-files --timeout 30 -e "ssh -p $PORT -o StrictHostKeyChecking=no" /home/data/ $USER@$IP:$DIR 2>&1)
    status=$?
    echo "$OUTPUT"

    CT=$(date +%H:%M)
    if [ "$CT" == "00:00" ] || [ "$CT" == "06:00" ] || [ "$CT" == "12:00" ] || [ "$CT" == "18:00" ]
    then
        echo "${OUTPUT##*$'\n'}"| telegram-send -g --stdin
    fi
    sleep $TIME
done
