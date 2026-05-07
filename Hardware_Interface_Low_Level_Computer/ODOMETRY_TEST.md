# Test odometry of AP4


## Run odometry test cases for gokart
There is test cases for the gokart to check if it behaves as it should. The tests are located in `Hardware_Interface_Low_Level_Computer\ap4_hwi_code\ap4hwi_ws\src\AP4_init_test_package\AP4_init_test_package`. The `ap4_distance_test.py` is a test that makes the gokart run for 5 meters and then stop, we then manually measure the distance the gokart travels. The other one contains steeringt as well where tyhe gokart runs for 2 meters straight, then turn 45 degrees and goes 2 more meters. In the terminal you will see final X, and Y-position and we can compare it to ground truth by measure it by hand. To execute these tests run this commands while ssh in to PI.

```bash
cd Desktop/GIT/autonomous_platform_generation_4/Hardware_Interface_Low_Level_Computer
docker exec ap4hwi bash -lc "
    cd ap4hwi_ws &&
    source /opt/ros/humble/setup.bash &&
    source install/setup.bash &&
    ros2 run AP4_init_test_package <testing_mode>
  "
```
Replace ```<testing_mode>``` with either `distance` or `turning` depending on which test you want to run.

## Run odometry test cases for gokart with IMU fusion using EKF
Launch ap4hlc container in **two** terminals on your local computer.

In HLC container 1:

```bash
  cd ~ap4_hlc_docker_dir/ap4_hlc_ws
  source install/setup.bash
  ros2 launch autonomous_platform_robot_description_pkg launch_robot.launch.py
```

***NOTE:*** you should not pick a 2D goal pose for this since we do not want to start autonomous navigation! Also make sure you have the correct ekf setting in launch file (ekf_imu.yaml/ekf.yaml)

In HLC container 2:

```bash
cd ~ap4_hlc_docker_dir/ap4_hlc_ws
source install/setup.bash
ros2 run odometry_test_pkg odometry_test
```
The odometry_test file will print raw odometry, filtered odometry and the difference between them. The data from the filtered odometry can be used to evaluate the tuning of the kalman filter.

Now ssh in to Pi and start test sequences by typing:

```bash
cd Desktop/GIT/autonomous_platform_generation_4/Hardware_Interface_Low_Level_Computer
docker exec ap4hwi bash -lc "
    cd ap4hwi_ws &&
    source /opt/ros/humble/setup.bash &&
    source install/setup.bash &&
    ros2 run AP4_init_test_package <test_mode>
  "
```
Change `<test_mode>` to either `turning` or `distance` depending on which test you want to run.

Compare the distance the gokart has traveled according to the `odometry_test` with measured ground truth distance. 