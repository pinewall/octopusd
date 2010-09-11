#!/bin/bash


HM=`dirname \`pwd\``
export NEWOCTOPUSD_HOME=$HM
if [ $# != 6 ]; then
echo octopustool ip port site oppdir_name data/name localdir_name
else
$HM/bin/octopustool $*
fi
#killall -9 newoctopusd 
#/home/bud/newoctopusd/bin/sendfile.sh 127.0.0.1 8123 auto /sendfile.sh1 1 /home/bud/newoctopusd/bin/sendfile.sh

