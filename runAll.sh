
make all

fileMissing=false

binDir="./bin/"

$binDir"DCMotor" L 1.8 0.02 0.004 0.02 3.2e-5 6.5e-6 0.01 &
$binDir"DCMotor" R 1.8 0.02 0.004 0.02 3.2e-5 6.5e-6 0.01 &

$binDir"RegPID" L 0.02 16 0.0 0.01 &
$binDir"RegPID" R 0.02 16 0.0 0.01 &

$binDir"Kinematics" 0.4 0.007 0.01 &

$binDir"Odometry" 0.01 &