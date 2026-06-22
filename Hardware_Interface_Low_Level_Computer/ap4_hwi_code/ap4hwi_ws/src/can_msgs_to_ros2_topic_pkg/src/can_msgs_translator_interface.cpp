#include <chrono>
#include <algorithm> //without this the std_msgs::msg could fail
#include <functional>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/float64.hpp"
#include "std_msgs/msg/int8.hpp"
#include "std_msgs/msg/int16.hpp"
#include "std_msgs/msg/u_int8.hpp"
#include "std_msgs/msg/u_int16.hpp"


// https://github.com/autowarefoundation/ros2_socketcan/blob/main/ros2_socketcan/src/socket_can_sender_node.cpp
//#include "ros2_socketcan/socket_can_common.hpp"
#include "can_msgs/msg/frame.hpp"

//#define __cplusplus
#include "include/CAN_DB.h"

using namespace std::chrono_literals;



/* This example creates a subclass of Node and uses std::bind() to register a
* member function as a callback from the timer. */

class TranslatorCANtoROS2Node : public rclcpp::Node
{
  public:
    TranslatorCANtoROS2Node()
    : Node("can_ros2_interface_node")
    {
        subscriber_incoming_can_msgs_ = this->create_subscription<can_msgs::msg::Frame>("/from_can_bus", 10, std::bind(&TranslatorCANtoROS2Node::ReadCanBusTopicCallback, this, std::placeholders::_1));
        publisher_outgoing_can_msgs_ = this->create_publisher<can_msgs::msg::Frame>("/to_can_bus", 10);
    
        /*
         * ====================================================================================================================
         * Signal publisher on unique topics
         * 
         * Every signal from the CAN Signal DATABASE will get it's own ROS topic.
         * The GET topics are used to represent the signals that have been published by other CAN Nodes. 
         * The SET topics are used to represent the signals that are to be written to other CAN nodes by this publisher. 
         * 
         * 
        */
        // Frame ID 100
        publisher_frame_100_Req_Heartbeat_= this->create_publisher<std_msgs::msg::Float64>("/GET_0x64_Req_Heartbeat", 10);

        // Frame ID 101
        publisher_frame_101_Response_Heartbeat_= this->create_publisher<std_msgs::msg::UInt8>("/GET_0x65_Response_Heartbeat",10);
        
        // Frame ID 500
        // todo
        // ...

        // Frame ID 1000
        publisher_frame_1000_Act_BreakVoltage_ = this->create_publisher<std_msgs::msg::UInt16>("/GET_0x3e8_Act_BreakVoltage", 10);
        publisher_frame_1000_Act_ThrottleVoltage_ = this->create_publisher<std_msgs::msg::UInt16>("/GET_0x3e8_Act_ThrottleVoltage", 10);
        publisher_frame_1000_Act_SteeringPosition_ = this->create_publisher<std_msgs::msg::Int8>("/GET_0x3e8_Act_SteeringPosition", 10);
        publisher_frame_1000_Act_SteeringVelocity_ = this->create_publisher<std_msgs::msg::UInt8>("/GET_0x3e8_Act_SteeringVelocity", 10);
        publisher_frame_1000_Act_Reverse_ = this->create_publisher<std_msgs::msg::UInt8>("/GET_0x3e8_Act_Reverse", 10);

        // Frame ID 1500
        publisher_frame_1500_SpeedSensorLF_PulseCnt_ = this->create_publisher<std_msgs::msg::UInt16>("/GET_0x5dc_SpeedSensorLF_PulseCnt", 10);
        publisher_frame_1500_SpeedSensorRF_PulseCnt_ = this->create_publisher<std_msgs::msg::UInt16>("/GET_0x5dc_SpeedSensorRF_PulseCnt", 10);
        publisher_frame_1500_SpeedSensorLR_PulseCnt_ = this->create_publisher<std_msgs::msg::UInt16>("/GET_0x5dc_SpeedSensorLR_PulseCnt", 10);
        publisher_frame_1500_SpeedSensorRR_PulseCnt_ = this->create_publisher<std_msgs::msg::UInt16>("/GET_0x5dc_SpeedSensorRR_PulseCnt", 10);
        publisher_frame_1500_SpeedSensorSampleTime_ = this->create_publisher<std_msgs::msg::UInt16>("/GET_0x5dc_SpeedSensorSampleTime", 10);

        // Frame ID 2000
        publisher_frame_2000_Get_SteeringAngle_ = this->create_publisher<std_msgs::msg::Int16>("/GET_0x7d0_Get_SteeringAngle", 10);
        publisher_frame_2000_Get_ReverseMode_ = this->create_publisher<std_msgs::msg::UInt8>("/GET_0x7d0_Get_ReverseMode", 10);

        //todo next frame
        //todo next signal
        //todo next signal


        /*
         * ====================================================================================================================
         * Signal subscribers on unique topics
        */
        // Frame ID 100
        subscriber_frame_100_Req_Heartbeat_ = this->create_subscription<std_msgs::msg::Float64>("/SET_0x64_Req_Heartbeat", 10, std::bind(&TranslatorCANtoROS2Node::Callback_frame_100_Req_Heartbeat, this, std::placeholders::_1));
        
        // Frame ID 101
        subscriber_frame_101_Response_Heartbeat_ = this->create_subscription<std_msgs::msg::UInt8>("/SET_0x65_Response_Heartbeat", 10, std::bind(&TranslatorCANtoROS2Node::Callback_frame_101_Response_Heartbeat, this, std::placeholders::_1));

        // Frame ID 500
        // todo
        // ...

        // Frame ID 1000
        subscriber_frame_1000_Act_BreakVoltage_ = this->create_subscription<std_msgs::msg::UInt16>("/SET_0x3e8_Act_BreakVoltage", 10, std::bind(&TranslatorCANtoROS2Node::Callback_frame_1000_Act_BreakVoltage, this, std::placeholders::_1));
        subscriber_frame_1000_Act_ThrottleVoltage_ = this->create_subscription<std_msgs::msg::UInt16>("/SET_0x3e8_Act_ThrottleVoltage", 10, std::bind(&TranslatorCANtoROS2Node::Callback_frame_1000_Act_ThrottleVoltage, this, std::placeholders::_1));
        subscriber_frame_1000_Act_SteeringPosition_ = this->create_subscription<std_msgs::msg::Int8>("/SET_0x3e8_Act_SteeringPosition", 10, std::bind(&TranslatorCANtoROS2Node::Callback_frame_1000_Act_SteeringPosition, this, std::placeholders::_1));
        subscriber_frame_1000_Act_SteeringVelocity_ = this->create_subscription<std_msgs::msg::UInt8>("/SET_0x3e8_Act_SteeringVelocity", 10, std::bind(&TranslatorCANtoROS2Node::Callback_frame_1000_Act_SteeringVelocity, this, std::placeholders::_1));
        subscriber_frame_1000_Act_Reverse_ = this->create_subscription<std_msgs::msg::UInt8>("/SET_0x3e8_Act_Reverse", 10, std::bind(&TranslatorCANtoROS2Node::Callback_frame_1000_Act_Reverse, this, std::placeholders::_1));
        
        // Frame ID 1500

        subscriber_frame_1500_SpeedSensorLF_PulseCnt_ = this->create_subscription<std_msgs::msg::UInt16>("/SET_0x5dc_SpeedSensorLF_PulseCnt", 10, std::bind(&TranslatorCANtoROS2Node::Callback_frame_1500_SpeedSensorLF_PulseCnt_, this, std::placeholders::_1));
        subscriber_frame_1500_SpeedSensorRF_PulseCnt_ = this->create_subscription<std_msgs::msg::UInt16>("/SET_0x5dc_SpeedSensorRF_PulseCnt", 10, std::bind(&TranslatorCANtoROS2Node::Callback_frame_1500_SpeedSensorRF_PulseCnt_, this, std::placeholders::_1));
        subscriber_frame_1500_SpeedSensorLR_PulseCnt_ = this->create_subscription<std_msgs::msg::UInt16>("/SET_0x5dc_SpeedSensorLR_PulseCnt", 10, std::bind(&TranslatorCANtoROS2Node::Callback_frame_1500_SpeedSensorLR_PulseCnt_, this, std::placeholders::_1));
        subscriber_frame_1500_SpeedSensorRR_PulseCnt_ = this->create_subscription<std_msgs::msg::UInt16>("/SET_0x5dc_SpeedSensorRR_PulseCnt", 10, std::bind(&TranslatorCANtoROS2Node::Callback_frame_1500_SpeedSensorRR_PulseCnt_, this, std::placeholders::_1));
        subscriber_frame_1500_SpeedSensorSampleTime_ = this->create_subscription<std_msgs::msg::UInt16>("/SET_0x5dc_SpeedSensorSampleTime", 10, std::bind(&TranslatorCANtoROS2Node::Callback_frame_1500_SpeedSensorSampleTime_, this, std::placeholders::_1));

        // Frame ID 2000
        subscriber_frame_2000_Get_SteeringAngle_ = this->create_subscription<std_msgs::msg::Int16>("/SET_0x7d0_Get_SteeringAngle", 10, std::bind(&TranslatorCANtoROS2Node::Callback_frame_2000_Get_SteeringAngle, this, std::placeholders::_1));
        subscriber_frame_2000_Get_ReverseMode_ = this->create_subscription<std_msgs::msg::UInt8>("/SET_0x7d0_Get_ReverseMode", 10, std::bind(&TranslatorCANtoROS2Node::Callback_frame_2000_Get_ReverseMode, this, std::placeholders::_1));

        //todo next frame
        //todo next signal
        //todo next signal
    }


  private:
   /*
   Define private functions
   */

    /*
    Callback function

    reads from topic /from_can_bus which comes from ros2_socketcan package

    Decode incoming frame using CAN_DB code and publish every signal within the frame on seperate topics
    */
    void ReadCanBusTopicCallback(const can_msgs::msg::Frame msg)
    {
        int id = msg.id;
        int dlc = msg.dlc;
        uint64_t data_to_pack = ((uint64_t)msg.data[7] << 56) | 
                    ((uint64_t)msg.data[6] << 48) | 
                    ((uint64_t)msg.data[5] << 40) | 
                    ((uint64_t)msg.data[4] << 32) | 
                    ((uint64_t)msg.data[3] << 24) | 
                    ((uint64_t)msg.data[2] << 16) | 
                    ((uint64_t)msg.data[1] << 8) | 
                    ((uint64_t)msg.data[0] << 0);


        unpack_message(&can_storage_container, id, data_to_pack, dlc, 0);

        PublishIncomingDataOnRosTopics(id);

    }

    /*
    Publishes CAN signal values onto specific topics
    */
    bool PublishIncomingDataOnRosTopics(const int frame_id)
    {

        switch(frame_id)
        {
            case CAN_ID_REQUEST_HEARTBEAT:
            {
                /*
                publish every can signal contained in frame with id = 100
                */

                /* 
                General procedure for each signal in can frame 
                1: decode data into variable
                2: publish data
            
                */    
                uint64_t temp_data;
                decode_can_0x064_Sig_Req_Heartbeat(&can_storage_container, &temp_data);
                std_msgs::msg::Float64 send_data; 
                send_data.data = temp_data;
                publisher_frame_100_Req_Heartbeat_->publish(send_data);  

                //todo next signals

                break;
            }
                
            case CAN_ID_RESPONSE_HEARTBEAT_SPCU:
                {
                /*
                publish every can signal contained in frame with id = 101
                */

                uint8_t temp_data;
                decode_can_0x065_Response_Heartbeat_sig(&can_storage_container, &temp_data);
                std_msgs::msg::UInt8 send_data; 
                send_data.data = temp_data;
                publisher_frame_101_Response_Heartbeat_->publish(send_data);  
                //todo next signals


                break;
                }
            case CAN_ID_ERROR_SPCU:
            {
                //todo

                break;
            }
            case CAN_ID_SET_SPCU:
            {
                /*
                publish every can signal contained in frame with id = 1000 = 0x3e8
                */

                uint16_t temp_data_Act_BreakVoltage; 
                decode_can_0x3e8_Act_BreakVoltage(&can_storage_container, &temp_data_Act_BreakVoltage);
                std_msgs::msg::UInt16 send_data_Act_BreakVoltage;
                send_data_Act_BreakVoltage.data = temp_data_Act_BreakVoltage;
                publisher_frame_1000_Act_BreakVoltage_->publish(send_data_Act_BreakVoltage);

                uint16_t temp_data_Act_ThrottleVoltage; 
                decode_can_0x3e8_Act_ThrottleVoltage(&can_storage_container, &temp_data_Act_ThrottleVoltage);
                std_msgs::msg::UInt16 send_data_Act_ThrottleVoltage;
                send_data_Act_ThrottleVoltage.data = temp_data_Act_ThrottleVoltage;
                publisher_frame_1000_Act_ThrottleVoltage_->publish(send_data_Act_ThrottleVoltage);


                int8_t temp_data_Act_SteeringPosition; 
                decode_can_0x3e8_Act_SteeringPosition(&can_storage_container, &temp_data_Act_SteeringPosition);
                std_msgs::msg::Int8 send_data_Act_SteeringPosition;
                send_data_Act_SteeringPosition.data = temp_data_Act_SteeringPosition;
                publisher_frame_1000_Act_SteeringPosition_->publish(send_data_Act_SteeringPosition);

                uint8_t temp_data_Act_SteeringVelocity; 
                decode_can_0x3e8_Act_SteeringVelocity(&can_storage_container, &temp_data_Act_SteeringVelocity);
                std_msgs::msg::UInt8 send_data_Act_SteeringVelocity;
                send_data_Act_SteeringVelocity.data = temp_data_Act_SteeringVelocity;
                publisher_frame_1000_Act_SteeringVelocity_->publish(send_data_Act_SteeringVelocity);

                uint8_t temp_data_Act_Reverse; 
                decode_can_0x3e8_Act_Reverse(&can_storage_container, &temp_data_Act_Reverse);
                std_msgs::msg::UInt8 send_data_Act_Reverse;
                send_data_Act_Reverse.data = temp_data_Act_Reverse;
                publisher_frame_1000_Act_Reverse_->publish(send_data_Act_Reverse);


                break;
            }

                /*
                publish every can signal contained in frame with id = 2000 = 0x7d0
                */

            case CAN_ID_GET_SPEED_SENSOR:
            {

                uint16_t temp_data_SpeedSensorLF_PulseCnt_;
                
                decode_can_0x5dc_SpeedSensorLF_PulseCnt(&can_storage_container, &temp_data_SpeedSensorLF_PulseCnt_);
                std_msgs::msg::UInt16 send_data_SpeedSensorLF_PulseCnt_;
                send_data_SpeedSensorLF_PulseCnt_.data = temp_data_SpeedSensorLF_PulseCnt_;
                publisher_frame_1500_SpeedSensorLF_PulseCnt_ ->publish(send_data_SpeedSensorLF_PulseCnt_);
                

                uint16_t temp_data_SpeedSensorRF_PulseCnt_;
                decode_can_0x5dc_SpeedSensorRF_PulseCnt(&can_storage_container, &temp_data_SpeedSensorRF_PulseCnt_);
                std_msgs::msg::UInt16 send_data_SpeedSensorRF_PulseCnt_;
                send_data_SpeedSensorRF_PulseCnt_.data = temp_data_SpeedSensorRF_PulseCnt_;
                publisher_frame_1500_SpeedSensorRF_PulseCnt_ ->publish(send_data_SpeedSensorRF_PulseCnt_);
                

                uint16_t temp_data_SpeedSensorLR_PulseCnt_;
                decode_can_0x5dc_SpeedSensorLR_PulseCnt(&can_storage_container, &temp_data_SpeedSensorLR_PulseCnt_);
                std_msgs::msg::UInt16 send_data_SpeedSensorLR_PulseCnt_;
                send_data_SpeedSensorLR_PulseCnt_.data = temp_data_SpeedSensorLR_PulseCnt_;
                publisher_frame_1500_SpeedSensorLR_PulseCnt_ ->publish(send_data_SpeedSensorLR_PulseCnt_);
                
                
                uint16_t temp_data_SpeedSensorRR_PulseCnt_;
                decode_can_0x5dc_SpeedSensorRR_PulseCnt(&can_storage_container, &temp_data_SpeedSensorRR_PulseCnt_);
                std_msgs::msg::UInt16 send_data_SpeedSensorRR_PulseCnt_;
                send_data_SpeedSensorRR_PulseCnt_.data = temp_data_SpeedSensorRR_PulseCnt_;
                publisher_frame_1500_SpeedSensorRR_PulseCnt_ ->publish(send_data_SpeedSensorRR_PulseCnt_);
                
                uint16_t temp_data_SpeedSensorSampleTime_;
                decode_can_0x5dc_SpeedSensorSampleTime(&can_storage_container, &temp_data_SpeedSensorSampleTime_);
                std_msgs::msg::UInt16 send_data_SpeedSensorSampleTime_;
                send_data_SpeedSensorSampleTime_.data = temp_data_SpeedSensorSampleTime_;
                publisher_frame_1500_SpeedSensorSampleTime_ ->publish(send_data_SpeedSensorSampleTime_);
                
                break;
            }

            case CAN_ID_GET_SPCU:
            {
                /*
                publish every can signal contained in frame with id = 2000 = 0x7d0
                */

                // int16_t temp_data_Get_SteeringAngle; 
                // decode_can_0x7d0_Get_SteeringAngle(&can_storage_container, &temp_data_Get_SteeringAngle);
                // std_msgs::msg::Int16 send_data_Get_SteeringAngle;
                // send_data_Get_SteeringAngle.data = temp_data_Get_SteeringAngle;
                // publisher_frame_2000_Get_SteeringAngle_->publish(send_data_Get_SteeringAngle);

                int16_t temp_data_Get_SteeringAngle; 
                decode_can_0x7d0_Get_SteeringAngle(&can_storage_container, &temp_data_Get_SteeringAngle);

                // Clamp the steering angle to actual kinematics
                temp_data_Get_SteeringAngle = std::max<int16_t>(-22, std::min<int16_t>(temp_data_Get_SteeringAngle, 22));

                std_msgs::msg::Int16 send_data_Get_SteeringAngle;
                send_data_Get_SteeringAngle.data = temp_data_Get_SteeringAngle;
                publisher_frame_2000_Get_SteeringAngle_->publish(send_data_Get_SteeringAngle);


                uint8_t temp_data_Get_ReverseMode; 
                decode_can_0x7d0_Get_ReverseMode(&can_storage_container, &temp_data_Get_ReverseMode);
                std_msgs::msg::UInt8 send_data_Get_ReverseMode;
                send_data_Get_ReverseMode.data = temp_data_Get_ReverseMode;
                publisher_frame_2000_Get_ReverseMode_->publish(send_data_Get_ReverseMode);

                break;
            }
                
            default:
                {
                /*
                ERROR id not implemented yet
                */
                RCLCPP_INFO_STREAM(this->get_logger(), " [ERROR] ID=" << frame_id << " is not implemented in PublishIncomingDataOnRosTopics yet - NO DATA PUBLISHED");
                return false;
                }
               
        }

        return true;
    }

    bool PublishCanFrameToCanNetwork(const int frame_id)
    {

        can_msgs::msg::Frame send_frame;
                
        uint64_t data_out = 0;

        pack_message(&can_storage_container, frame_id, &data_out);

        send_frame.data[7] = data_out >> 56;
	    send_frame.data[6] = data_out >> 48;
	    send_frame.data[5] = data_out >> 40;
	    send_frame.data[4] = data_out >> 32;
	    send_frame.data[3] = data_out >> 24;
	    send_frame.data[2] = data_out >> 16;
	    send_frame.data[1] = data_out >>  8;
	    send_frame.data[0] = data_out >>  0;

        send_frame.id = frame_id;
        send_frame.dlc = 8;

        publisher_outgoing_can_msgs_->publish(send_frame);
        
        return true;
    }
    /*
    ====================================================================================================================
    Start of list of Callback functions for each signal subscriber topic

    Naming convention of subscriber callback function
    void Callback_frame_<ID>_<SIGNAL NAME>(const SIGNAL-TYPE msg)

    */

    // Callbacks for signals in Frame with ID = 100
    void Callback_frame_100_Req_Heartbeat(const std_msgs::msg::Float64 msg)
    {
        encode_can_0x064_Sig_Req_Heartbeat(&can_storage_container, (uint64_t)msg.data);

        PublishCanFrameToCanNetwork(CAN_ID_REQUEST_HEARTBEAT); // this signal is part of frame with ID = 0x64 = 100 in base 10
    }

    // Callbacks for signals in Frame with ID = 101
    void Callback_frame_101_Response_Heartbeat(const std_msgs::msg::UInt8 msg)
    {
        encode_can_0x065_Response_Heartbeat_sig(&can_storage_container, (uint8_t)msg.data);

        PublishCanFrameToCanNetwork(CAN_ID_RESPONSE_HEARTBEAT_SPCU); // this signal is part of frame with ID = 0x65 = 101 in base 10
    }

    // Callbacks for signals in Frame with ID = 500
    // todo
    // ...

    // Callbacks for signals in Frame with ID = 1000
    void Callback_frame_1000_Act_BreakVoltage(const std_msgs::msg::UInt16 msg)
    {
        encode_can_0x3e8_Act_BreakVoltage(&can_storage_container, msg.data);
        PublishCanFrameToCanNetwork(CAN_ID_SET_SPCU);
    }
    void Callback_frame_1000_Act_ThrottleVoltage(const std_msgs::msg::UInt16 msg)
    {
        encode_can_0x3e8_Act_ThrottleVoltage(&can_storage_container, msg.data);
        PublishCanFrameToCanNetwork(CAN_ID_SET_SPCU);
    }
    void Callback_frame_1000_Act_SteeringPosition(const std_msgs::msg::Int8 msg)
    {
        encode_can_0x3e8_Act_SteeringPosition(&can_storage_container, msg.data);
        PublishCanFrameToCanNetwork(CAN_ID_SET_SPCU);
    }
    void Callback_frame_1000_Act_SteeringVelocity(const std_msgs::msg::UInt8 msg)
    {
        encode_can_0x3e8_Act_SteeringVelocity(&can_storage_container, msg.data);
        PublishCanFrameToCanNetwork(CAN_ID_SET_SPCU);
    }
    void Callback_frame_1000_Act_Reverse(const std_msgs::msg::UInt8 msg)
    {
        encode_can_0x3e8_Act_Reverse(&can_storage_container, msg.data);
        PublishCanFrameToCanNetwork(CAN_ID_SET_SPCU);
    }
    
    // Callbacks for signals in Frame with ID = 1500
   
   // Needs a callback for every signal in the frame

    void Callback_frame_1500_SpeedSensorLF_PulseCnt_(const std_msgs::msg::UInt16 msg)
    {
        encode_can_0x5dc_SpeedSensorLF_PulseCnt(&can_storage_container, msg.data);
        PublishCanFrameToCanNetwork(CAN_ID_GET_SPEED_SENSOR);
    }

    void Callback_frame_1500_SpeedSensorRF_PulseCnt_(const std_msgs::msg::UInt16 msg)
    {
        encode_can_0x5dc_SpeedSensorRF_PulseCnt(&can_storage_container, msg.data);
        
        PublishCanFrameToCanNetwork(CAN_ID_GET_SPEED_SENSOR);
    }
 

    void Callback_frame_1500_SpeedSensorLR_PulseCnt_(const std_msgs::msg::UInt16 msg)
    {
        encode_can_0x5dc_SpeedSensorLR_PulseCnt(&can_storage_container, msg.data);
        
        PublishCanFrameToCanNetwork(CAN_ID_GET_SPEED_SENSOR);
    }


    void Callback_frame_1500_SpeedSensorRR_PulseCnt_(const std_msgs::msg::UInt16 msg)
    {
        encode_can_0x5dc_SpeedSensorRR_PulseCnt(&can_storage_container, msg.data);
        
        PublishCanFrameToCanNetwork(CAN_ID_GET_SPEED_SENSOR);
    }

    void Callback_frame_1500_SpeedSensorSampleTime_(const std_msgs::msg::UInt16 msg)
    {
        encode_can_0x5dc_SpeedSensorSampleTime(&can_storage_container, msg.data);
        
        PublishCanFrameToCanNetwork(CAN_ID_GET_SPEED_SENSOR);
    }


    // Callbacks for signals in Frame with ID = 2000
    void Callback_frame_2000_Get_SteeringAngle(const std_msgs::msg::Int16 msg)
    {
        encode_can_0x7d0_Get_SteeringAngle(&can_storage_container, msg.data);
        PublishCanFrameToCanNetwork(CAN_ID_GET_SPCU);
    }
    void Callback_frame_2000_Get_ReverseMode(const std_msgs::msg::UInt8 msg)
    {
        encode_can_0x7d0_Get_ReverseMode(&can_storage_container, msg.data);
        PublishCanFrameToCanNetwork(CAN_ID_GET_SPCU);
    }


    /*
    End of list of callback functions for each signal subscriber topic
    ====================================================================================================================
    
    */
    
    // decleare private variables
    rclcpp::Subscription<can_msgs::msg::Frame>::SharedPtr subscriber_incoming_can_msgs_;
    rclcpp::Publisher<can_msgs::msg::Frame>::SharedPtr publisher_outgoing_can_msgs_;

    // publish CAN signals on ros2 topics
    // Naming convention to use for future signals: publisher_frame_<ID>_<signal name>_
    
    // Frame ID 100
    rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr publisher_frame_100_Req_Heartbeat_;
    
    // Frame ID 101
    rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr publisher_frame_101_Response_Heartbeat_;
    
    // Frame ID 500
    //todo signals
    // ..
    
    // Frame ID 1000
    rclcpp::Publisher<std_msgs::msg::UInt16>::SharedPtr publisher_frame_1000_Act_BreakVoltage_;
    rclcpp::Publisher<std_msgs::msg::UInt16>::SharedPtr publisher_frame_1000_Act_ThrottleVoltage_;
    rclcpp::Publisher<std_msgs::msg::Int8>::SharedPtr publisher_frame_1000_Act_SteeringPosition_;
    rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr publisher_frame_1000_Act_SteeringVelocity_;
    rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr publisher_frame_1000_Act_Reverse_;

    // Frame ID 1500
    rclcpp::Publisher<std_msgs::msg::UInt16>::SharedPtr publisher_frame_1500_SpeedSensorLF_PulseCnt_;
    rclcpp::Publisher<std_msgs::msg::UInt16>::SharedPtr publisher_frame_1500_SpeedSensorRF_PulseCnt_;
    rclcpp::Publisher<std_msgs::msg::UInt16>::SharedPtr publisher_frame_1500_SpeedSensorLR_PulseCnt_;
    rclcpp::Publisher<std_msgs::msg::UInt16>::SharedPtr publisher_frame_1500_SpeedSensorRR_PulseCnt_;
    rclcpp::Publisher<std_msgs::msg::UInt16>::SharedPtr publisher_frame_1500_SpeedSensorSampleTime_;
    
    // Frame ID 2000
    rclcpp::Publisher<std_msgs::msg::Int16>::SharedPtr publisher_frame_2000_Get_SteeringAngle_;
    rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr publisher_frame_2000_Get_ReverseMode_;

    // subscribe to each ros2 signal topic
    // Naming convention to use for future signals: subscriber_frame_<ID>_<signal name>_
    
    // Frame ID 100
    rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr subscriber_frame_100_Req_Heartbeat_;
    
    // Frame ID 101
    rclcpp::Subscription<std_msgs::msg::UInt8>::SharedPtr subscriber_frame_101_Response_Heartbeat_;

    // Frame ID 500
    //todo signals
    // ..
    
    // Frame ID 1000
    rclcpp::Subscription<std_msgs::msg::UInt16>::SharedPtr subscriber_frame_1000_Act_BreakVoltage_;
    rclcpp::Subscription<std_msgs::msg::UInt16>::SharedPtr subscriber_frame_1000_Act_ThrottleVoltage_;
    rclcpp::Subscription<std_msgs::msg::Int8>::SharedPtr subscriber_frame_1000_Act_SteeringPosition_;
    rclcpp::Subscription<std_msgs::msg::UInt8>::SharedPtr subscriber_frame_1000_Act_SteeringVelocity_;
    rclcpp::Subscription<std_msgs::msg::UInt8>::SharedPtr subscriber_frame_1000_Act_Reverse_;

    // Frame ID 1500
    rclcpp::Subscription<std_msgs::msg::UInt16>::SharedPtr subscriber_frame_1500_SpeedSensorLF_PulseCnt_;
    rclcpp::Subscription<std_msgs::msg::UInt16>::SharedPtr subscriber_frame_1500_SpeedSensorRF_PulseCnt_;
    rclcpp::Subscription<std_msgs::msg::UInt16>::SharedPtr subscriber_frame_1500_SpeedSensorLR_PulseCnt_;
    rclcpp::Subscription<std_msgs::msg::UInt16>::SharedPtr subscriber_frame_1500_SpeedSensorRR_PulseCnt_;
    rclcpp::Subscription<std_msgs::msg::UInt16>::SharedPtr subscriber_frame_1500_SpeedSensorSampleTime_;

    // Frame ID 2000
    rclcpp::Subscription<std_msgs::msg::Int16>::SharedPtr subscriber_frame_2000_Get_SteeringAngle_;
    rclcpp::Subscription<std_msgs::msg::UInt8>::SharedPtr subscriber_frame_2000_Get_ReverseMode_;

    /*
    ========================================================================================================
    */
    // can storage container object from CAN_DB library, (derived from DBC to C autogenerated lib) store data for autonomous platform in this object
    can_obj_can_db_h_t can_storage_container;
    
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TranslatorCANtoROS2Node>());
  rclcpp::shutdown();
  return 0;
}
