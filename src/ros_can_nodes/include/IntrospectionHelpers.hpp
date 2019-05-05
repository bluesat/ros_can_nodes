/**
 * Date Started:
 * Original Author: Yiwei Han
 * Editors:
 * ROS Node Name:
 * ROS Package: ros_can_nodes
 * Purpose: Helper module for manipulating raw ROS message data
 * This code is released under the MIT License. Copyright BLUEsat UNSW, 2019
 */

#ifndef INTROSPECTIONHELPERS_HPP
#define INTROSPECTIONHELPERS_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <ros_type_introspection/utils/shape_shifter.hpp>

namespace IntrospectionHelpers {
    /**
     * Register message type for ros message type introspection
     * Message registration is thread safe
     */
    void register_message(const RosIntrospection::ShapeShifter::ConstPtr& msg, const std::string& topic_name);

    /**
     * Print all registered message types and fields structures
     */
    void print_registered();

    /**
     * Modifies a ROS message data buffer for communication on the CAN bus
     * datatype must be a registered message type
     */
    std::vector<uint8_t> modify_buffer(const std::string& datatype, const uint8_t *const data, const uint32_t size);
}

#endif // INTROSPECTIONHELPERS_HPP