/*
 * Date Started: 20/10/2017
 * Original Author: Simon Ireland
 * Editors:
 * ROS Node Name:
 * ROS Package: ros_can_nodes
 * Purpose:
 * This code is released under the MIT  License. Copyright BLUEsat UNSW, 2017
 */

#ifndef TOPICBUFFERS_H
#define TOPICBUFFERS_H

#include <unordered_map>
#include <cstdint>


#define MAX_CAN_MSGS 32
#define TOPIC_BUFFER_SIZE (MAX_CAN_MSGS)

typedef uint8_t buffer[TOPIC_BUFFER_SIZE];

// NOTE: above line is bad style, as calling sizeof will produce incorrect
// results if not aware of the array type of 'buffer'

class TopicBuffers {
    public:
        static TopicBuffers& instance();

        void initBuffers();

        void appendData(short key, uint8_t data);

        void processData(short key, uint8_t* data, int d_len, bool last_msg);

        // TODO: Possible Custom Hash function for unordered_map
        // TODO: Possible Custom Equality function for unordered_map
    private:
        TopicBuffers() {}

        std::unordered_map<short, buffer> topic_buffers;

        TopicBuffers(const TopicBuffers&) = delete;
        void operator=(const TopicBuffers&) = delete;
};
#endif // TOPICBUFFERS_H
