/*
 * Date Started: 20/10/2017
 * Original Author: Simon Ireland
 * Editors:
 * ROS Node Name:
 * ROS Package: ros_can_nodes
 * Purpose:
 * This code is released under the MIT  License. Copyright BLUEsat UNSW, 2017
 */

#include "CANBuffers.hpp"
#include <vector>
#include <cstdint>
#include <iostream>
#include <ros/console.h>

void CANBuffers::reset(const uint16_t key, const uint8_t expected_frames) {
    auto& buffer = buffers[key];
    ROS_INFO("CAN buffers: resetting for key %d", key);
    buffer.buf.clear();
    buffer.expected_frames = expected_frames;
    buffer.received_frames = 0;
}

void CANBuffers::append(const uint16_t key, const uint8_t data[CAN_MAX_DLEN], const uint8_t data_len) {
    auto& buffer = buffers[key];
    ROS_INFO("CAN buffers: appending %d bytes to key %d", data_len, key);
    buffer.buf.insert(buffer.buf.cend(), data, data + data_len);
    ++buffer.received_frames;
}

bool CANBuffers::ready(const uint16_t key) {
    auto& buffer = buffers[key];
    return buffer.expected_frames == buffer.received_frames;
}

const std::vector<uint8_t>& CANBuffers::get(const uint16_t key) {
    return buffers[key].buf;
}
