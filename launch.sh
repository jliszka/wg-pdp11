#!/bin/sh

# run SimH and the java panel simulation
cd $(dirname $0)

echo "Starting panel simulation and portmapper ..."

# Select a start width
java -classpath bin/panelsim_all.jar blinkenbone.panelsim.panelsim1170.Panelsim1170_app --width 1000  --addr_select 0 --data_select 1 &
JAVAPID=$!

echo "Wait 15 seconds for java/portmap to come up ..."

sleep 15

echo "Starting SimH ..."
(
  ./bin/pdp11_realcons $1
  kill $JAVAPID
)
