#!/bin/bash

CheckProcess()
{
    if [ "$1" = "" ];
    then
        return 1
    fi

    PROCESS_NUM=`ps -ef | grep "$1" | grep -v "grep" |grep -v "sh"| wc -l`
    echo $PROCESS_NUM
    if [ $PROCESS_NUM -ge 1 ];
    then
        return 0
    else
        return 1
    fi
}

CheckProcess "newoctopusd"
CheckQQ_RET=$?
HM=`dirname \`pwd\``
export NEWOCTOPUSD_HOME=$HM
if [ $CheckQQ_RET -eq 1 ];
then
    killall -9 "newoctopusd"
    $HM/bin/start.sh
fi

