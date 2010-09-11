#!/bin/bash


HM=`dirname \`pwd\``
export NEWOCTOPUSD_HOME=$HM
$HM/bin/octopustool 5 n
#killall -9 newoctopusd 
