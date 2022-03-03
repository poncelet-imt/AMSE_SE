binDir="./bin/"

resultDir="./result/"

odometryFile=$resultDir"dataOdometry.csv"
kinematicsFile=$resultDir"dataKinematics.csv"

make all

touch $odometryFile | cat >$odometryFile
touch $kinematicsFile | cat >$kinematicsFile

K="0.009"
I="13.5"
Te="0.01"
tMax="5.0"

$binDir"ResetState" L

$binDir"DCMotor" L 1.8 0.02 0.004 0.02 3.2e-5 6.5e-6 $Te $tMax >> /dev/null &
pidDCMotor1=$!
$binDir"DCMotor" R 1.8 0.02 0.004 0.02 3.2e-5 6.5e-6 $Te $tMax >> /dev/null &
pidDCMotor1=$!

$binDir"RegPID" L $K $I 0.0 $Te $tMax > /dev/null &
pidRegPID1=$!
$binDir"RegPID" R $K $I 0.0 $Te $tMax > /dev/null &
pidRegPID2=$!

$binDir"Kinematics" 0.4 0.007 $Te $tMax >> $kinematicsFile &
pidKinematics=$!

$binDir"Odometry" $Te $tMax >> $odometryFile &
pidOdometry=$!

$binDir"SetTv" L 2.0 &
$binDir"SetTv" R 1.0 &


wait $pidDCMotor1 2> /dev/null
wait $pidDCMotor2 2> /dev/null
wait $pidRegPID1 2> /dev/null
wait $pidRegPID2 2> /dev/null
wait $pidKinematics 2> /dev/null
wait $pidOdometry 2> /dev/null

python3 ./graphPloter.py $odometryFile