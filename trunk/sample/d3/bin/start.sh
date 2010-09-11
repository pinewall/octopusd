#!/bin/bash


#ulimit -c unlimited 
ulimit -c 10240

HM=`dirname \`pwd\``
export NEWOCTOPUSD_HOME=$HM
$HM/bin/newoctopusd
