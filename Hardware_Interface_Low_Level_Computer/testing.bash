#!/bin/bash
sudo ip link set can0 up type can bitrate 1000000
sudo ip link set can0 txqueuelen 1000
echo "CAN initalised"
docker-compose down
docker-compose build
docker-compose up -d

# HE: TODO: If checking the output validity is not possible, maybe mention the expected output in comments
