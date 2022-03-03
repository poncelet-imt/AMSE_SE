#!/bin/bash
binDir="./bin/"

resultDir="./result/"

odometryFile=$resultDir"dataOdometry.csv"
kinematicsFile=$resultDir"dataKinematics.csv"

make all

touch $odometryFile | cat >$odometryFile
touch $kinematicsFile | cat >$kinematicsFile

PI="3.14"


K="0.009"
I="13.5"
Te="0.01"
tMax="5.0"

$binDir"ResetState" L >> /dev/null
$binDir"ResetState" R >> /dev/null

$binDir"DCMotor" L 1.8 0.02 0.004 0.02 3.2e-5 6.5e-6 $Te >> /dev/null &
pidDCMotor1=$!
$binDir"DCMotor" R 1.8 0.02 0.004 0.02 3.2e-5 6.5e-6 $Te >> /dev/null &
pidDCMotor2=$!

$binDir"RegPID" L $K $I 0.0 $Te > /dev/null &
pidRegPID1=$!
$binDir"RegPID" R $K $I 0.0 $Te > /dev/null &
pidRegPID2=$!

$binDir"Kinematics" 0.4 0.007 $Te >> $kinematicsFile &
pidKinematics=$!

$binDir"Odometry" $Te >> $odometryFile &
pidOdometry=$!

sh ./program.sh &
pidProgram=$!


wait $pidProgram

sleep 2

kill -10 $pidDCMotor1
kill -10 $pidDCMotor2
kill -10 $pidRegPID1
kill -10 $pidRegPID2
kill -10 $pidKinematics
kill -10 $pidOdometry

python3 ./graphPloter.py $odometryFile --xy

python3 ./graphPloter.py $odometryFile
