/*
 * Date Started: 29/09/2017
 * Original Author: Simon Ireland
 * Editors:
 * ROS Node Name: ros_can_nodes
 * ROS Package: ros_can_nodes
 * Purpose: Main entry point of ros_can_nodes. Run with:
 *          - `rosrun ros_can_nodes ros_can_nodes` to use can0 as the CAN port
 *          - `rosrun ros_can_nodes ros_can_nodes vcan0` to use vcan0 as the CAN port
 * This code is released under the BSD License. Copyright BLUEsat UNSW, 2017
 */

#ifndef CAN_MSG_ROUTER_HPP
#define CAN_MSG_ROUTER_HPP

#include <linux/can.h>
#include <string>
#include <vector>
#include <utility>
#include "CANBuffers.hpp"

class CANMsgRouter {

    public:
        static void init(const std::string& can_port);

        static void run();

        static void publisherTest();

        static void subscriberTest();

        static void processCANMsg(const can_frame& msg);

        static void routeControlMsg(const can_frame& msg);

        static void routePublishMsg(const can_frame& msg);

        static void extractTopic(const can_frame& first, std::string& topic, std::string& topic_type);

        /**
         * Extracts and returns the topic and topic type as strings from the given buffer
         */
        static std::pair<std::string, std::string> extractTopic(const std::vector<uint8_t>& buf);

        static void topicRegisterHelper(const uint8_t mode, const can_frame& msg);

        /**
         * Resets all possible nodes on the channel
         */
        static void resetAllNodes();
    private:
        static CANBuffers topic_register_buffers;
        static CANBuffers publish_buffers;
};

#endif // CAN_MSG_ROUTER_HPP
