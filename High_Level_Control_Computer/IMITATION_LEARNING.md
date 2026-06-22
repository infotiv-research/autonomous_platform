# Imitation Learning

### Imitation Learning Network

To collect data from training and test drive rosbag2 is used. The data collection is done in the High Level Planner Docker and specifically in the folder bagfiles_2024, within source. With the High Level Control Docker running, the directory is accessed as follows.

```bash
docker exec -it ap4hlc bash
source rosbag_startup.bash
```

or

```bash
docker exec -it ap4hlc bash
cd ap4hlc_ws
source install/setup.bash
cd ap4hlc_ws/src/bagfiles_2024
```

To record topics from the physical Go-Kart you will have to be in ROS_DOMAIN_ID=1 such as

```bash
export ROS_DOMAIN_ID=1
```

When data is published to the topics which you want to record data from (probably /cmd_vel, /color/image, /stereo/depth, /imu)

```bash
ros2 bag record cmd_vel_stamped color/image stereo/depth imu
```

The recorded data can be replayed as, replacing <bag> with the desired bag:

```bash
ros2 bag play <bag>
```

The playback can also be played as a loop

```bash
ros2 bag play --loop <bag>
```

Empty bagfiles directory

```bash
rm -R -- */
```

to delete all files in a directory wihtout deleting any folders run:

```bash
find . -type f -delete
```

Instead of saving the data to a rosbag and then play it back creating a .pkl file it can also directly be recorded into a .pkl file using the following command:

```bash
docker exec -it ap4hlc bash
source data_collection_startup.bash
```

To train the network using the collected data, the data is written to a .pkl file as this allows training the network fully outside of ROS. This is done by playing the desired rosbag while running the file data_collection.py. It is important to start the data_collection file before starting playing the rosbag. It is also important to play the rosbag with loop for it to be able to collect all the messages.

The subscriber queue deletes the messages with the oldest timestamp first. Therefore no messages will be collected on the second run of the loop.

The `data_collection.py` file is run in the High Level Computer docker as:

```bash
ros2 run autonomous_platform_robot_description_pkg data_collection.py 
```

The .pkl file can then be used as input within train_DAgger.py, found in `/autonomous_platform/High_Level_Control_Computer/ap4_hlc_code/ap4hlc_ws/src/imitation_learning/train_DAgger.py`

### OAK-D camera node

Before running the OAK-D camera node, build the docker with:

```bash
docker build --build-arg USE_RVIZ=1 -t depthai-ros .
```

To start OAK-D camera nodes enter the High_Level_Control_Computer directory and run:

```bash
"./Imitation learning/launch_depthai_ros.sh"
```

This will launch the camera at the resolution 208 x  156, the resolution can be change by changing rgbScaleDinominator and rgbScaleNumerator in 'launch_depthai_ros.sh', however there is a latancy at higher resolutions for the depth image. Possible resolutions can be seen in the table below:

```
4056 x 3040 *  2/13 -->  624 x  468
4056 x 3040 *  2/39 -->  208 x  156
4056 x 3040 *  2/51 -->  160 x  120
4056 x 3040 *  4/13 --> 1248 x  936
4056 x 3040 *  4/26 -->  624 x  468
4056 x 3040 *  4/29 -->  560 x  420
4056 x 3040 *  4/35 -->  464 x  348
4056 x 3040 *  4/39 -->  416 x  312
4056 x 3040 *  6/13 --> 1872 x 1404
4056 x 3040 *  6/39 -->  624 x  468
4056 x 3040 *  7/25 --> 1136 x  852
4056 x 3040 *  8/26 --> 1248 x  936
4056 x 3040 *  8/39 -->  832 x  624
4056 x 3040 *  8/52 -->  624 x  468
4056 x 3040 *  8/58 -->  560 x  420
4056 x 3040 * 10/39 --> 1040 x  780
4056 x 3040 * 10/59 -->  688 x  516
4056 x 3040 * 12/17 --> 2864 x 2146
4056 x 3040 * 12/26 --> 1872 x 1404
4056 x 3040 * 12/39 --> 1248 x  936
4056 x 3040 * 13/16 --> 3296 x 2470
4056 x 3040 * 14/39 --> 1456 x 1092
4056 x 3040 * 14/50 --> 1136 x  852
4056 x 3040 * 14/53 --> 1072 x  804
4056 x 3040 * 16/39 --> 1664 x 1248
4056 x 3040 * 16/52 --> 1248 x  936
```