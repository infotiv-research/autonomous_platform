#!/bin/bash

cd ap4hlc_ws
source install/setup.bash

echo -e "\nSetting ros domain to 1"
export ROS_DOMAIN_ID=1

# IP address of RPi
RPi_IP_ADDRESS_WIFI="192.168.150.172"
RPi_IP_ADDRESS_ETHERNET="192.168.0.156"
RPi_ROUTER_IP_ADDRESS="192.168.0.1"

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'
RPi_ping=0
Router_ping=0
while true; do
    RPi_ping_status=""
    Router_ping_status=""
    # echo "--------------------------------------------------------------------"
    # echo "Starting network communication test for WIFI..."
    # echo "--------------------------------------------------------------------"
    # # Test connectivity using ping
    # echo "Checking network communication with RPi..."
    # ping -c 4 $RPi_IP_ADDRESS_WIFI
    # echo "--------------------------------------------------------------------"
    # # Check open ports using netcat (example checks port 80)
    # echo "Checking if port 80 is open on $RPi_IP_ADDRESS_WIFI..."
    # nc -zv $RPi_IP_ADDRESS_WIFI 80
    # echo "--------------------------------------------------------------------"
    # # Trace route to the host
    # echo "Tracing route to RPi at ip adress: $RPi_IP_ADDRESS_WIFI..."
    # traceroute $RPi_IP_ADDRESS_WIFI





    # echo -e "\n--------------------------------------------------------------------"

    # echo "Starting network communication test RPi..."
    # echo -e "--------------------------------------------------------------------\n"
    # # Test connectivity using ping
    # echo "Checking network communication with RPi..."
    # if ping -c 4 $RPi_IP_ADDRESS_ETHERNET; then
    #     echo "Ping to RPi successful"
    #     RPi_ping_status="Success"
    #     RPi_ping=1
    # else
    #     echo "Failed to ping RPi"
    #     RPi_ping_status="Failed"
    #     RPi_ping=0
    # fi
    # echo "--------------------------------------------------------------------"
    # # Trace route to the host
    # echo -e "\nTracing route to RPi at ip adress: $RPi_IP_ADDRESS_ETHERNET..."
    # traceroute $RPi_IP_ADDRESS_ETHERNET

    # echo -e "\n--------------------------------------------------------------------"
    # echo "Starting network communication test to ROUTER..."
    # echo -e "--------------------------------------------------------------------\n"
    # # Test connectivity using ping
    # echo "Checking network communication with AP4 ROUTER..."
    # if ping -c 4 $RPi_ROUTER_IP_ADDRESS; then
    #     echo "Ping to Router successful"
    #     Router_ping_status="Success"
    #     Router_ping=1
    # else
    #     Router_ping=0
    #     echo "Failed to ping Router"
    #     Router_ping_status="Failed"
    # fi
    # echo -e "--------------------------------------------------------------------\n"
    # # Check open ports using netcat (example checks port 80)
    # echo "Checking if port 80 is open on $RPi_ROUTER_IP_ADDRESS..."
    # nc -zv $RPi_ROUTER_IP_ADDRESS 80


    ls

    echo "--------------------------------------------------------------------"
    echo "Network communication test is done."

    echo -e "\n--------------------------------------------------------------------"
    echo "Starting ROS communication test..."
    echo -e "--------------------------------------------------------------------\n"


    echo "Listing and printing active ROS topics..."
    sleep 2
    ros2 topic list
    sleep 1
    echo -e "--------------------------------------------------------------------\n"

    echo -e "\nComparing current ROS topics with happy case..."

    # Capture the current state of ROS topics
    ros2 topic list > ../test_topic_lists/current_topics.txt

    # Compare the current state with the happy case
    diff ../test_topic_lists/happy_case_topics.txt ../test_topic_lists/current_topics.txt > ../test_topic_lists/topics_diff.txt

    # Check if there were differences
    if [ -s ../test_topic_lists/topics_diff.txt ]; then
        echo "Differences found in ROS topics:"
        cat ../test_topic_lists/topics_diff.txt
    else
        echo "No differences found. All expected topics are active."
    fi
    echo "--------------------------------------------------------------------"

    
    echo -e "\nListing active ROS nodes..."
    sleep 2
    ros2 node list
    sleep 1
    echo "--------------------------------------------------------------------"
    echo -e "\nComparing current ROS nodes with happy case..."

    # Capture the current state of ROS topics
    ros2 node list > ../test_topic_lists/current_nodes.txt

    # Compare the current state with the happy case
    diff ../test_topic_lists/happy_case_nodes.txt ../test_topic_lists/current_nodes.txt > ../test_topic_lists/nodes_diff.txt

    # Check if there were differences
    if [ -s ../test_topic_lists/nodes_diff.txt ]; then
        echo "Differences found in ROS nodes:"
        cat ../test_topic_lists/nodes_diff.txt
    else
        echo "No differences found. All expected nodes are active."
    fi
    echo -e "--------------------------------------------------------------------\n"

    echo -e "${GREEN}All tests done, outcome of test:\n${NC}"

    echo "--------------------------------------------------------------------"
    if [ -s ../test_topic_lists/topics_diff.txt ]; then
        echo -e "${RED}Topics lost are: \n"
        cat ../test_topic_lists/topics_diff.txt
        echo -e "${NC}"
    else
        echo -e "\n${GREEN}All expected topics are active.${NC}"
    fi
    echo "--------------------------------------------------------------------"
    if [ -s ../test_topic_lists/nodes_diff.txt ]; then
        echo -e "${RED}Nodes lost are: \n"
        cat ../test_topic_lists/nodes_diff.txt
        echo -e "${NC}"
    else
        echo -e  "\n${GREEN}All expected nodes are active.${NC}"
    fi
    echo -e "--------------------------------------------------------------------\n"

    if [ $RPi_ping -eq 1 ]; then
    echo -e "${GREEN}RPi Ping Status: $RPi_ping_status${NC}"
    else 
        echo -e "${RED}RPi Ping Status: $RPi_ping_status${NC}"
    fi

    if [ $Router_ping -eq 1 ]; then
        echo -e "${GREEN}Router Ping Status: $Router_ping_status${NC}"
    else
        echo -e "${RED}Router Ping Status: $Router_ping_status${NC}"
    fi

    echo -e "--------------------------------------------------------------------\n"

    sleep 30
done