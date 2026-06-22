The matlab file in this folder was develop and used for plotting and comparing raw odometry, EKF and ground trouth in form of mocap file. 

This is done by opening the file in matlab and adding the files you want to plot against eachother in the same folder that was opened in matlab. The odometry and the EKF has to be in the form of a rosbag. While the mocap data has to be in the form of a matlab datafile. If no mocap data is available one can disable it in the script and run with just the other two datasets.

The rosbag can be recorded manually or with the the help of the script that also starts the slam that can be run with the command ./start_and_record.