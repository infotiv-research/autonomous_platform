#!/bin/bash

flag=${1:-1}


cd ap4hlc_ws/
source install/setup.bash

cd src/imitation_learning/


echo "Starting train_DAgger.py"
python3 train_DAgger.py --param ${flag}


