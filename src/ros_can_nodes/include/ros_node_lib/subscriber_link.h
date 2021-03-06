/*
 * Copyright (C) 2008, Morgan Quigley and Willow Garage, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the names of Stanford University or Willow Garage, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ROSCAN_SUBSCRIBER_LINK_H
#define ROSCAN_SUBSCRIBER_LINK_H

#include "ros_node_lib/common.h"
#include <cstdint>
#include <string>
#include <vector>
#include <typeinfo>
#include <ros/serialized_message.h>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace roscan {

class SubscriberLink : public boost::enable_shared_from_this<SubscriberLink> {
    public:
        class Stats {
            public:
                uint64_t bytes_sent_, message_data_sent_, messages_sent_;
                Stats() : bytes_sent_{0}, message_data_sent_{0}, messages_sent_{0} {}
        };

        SubscriberLink(const RosNodePtr& node) : node_{node}, connection_id_{0} {}
        virtual ~SubscriberLink() {}

        const std::string& getTopic() const { return topic_; }
        const Stats& getStats() const { return stats_; }
        const std::string& getDestinationCallerID() const { return destination_caller_id_; }
        int getConnectionID() const { return connection_id_; }

        // Queue up a message for publication.  Throws out old messages if we've reached our Publication's max queue size
        virtual void enqueueMessage(const ros::SerializedMessage& m, const bool ser, const bool nocopy) = 0;

        virtual void drop() = 0;

        virtual std::string getTransportType() = 0;
        virtual std::string getTransportInfo() = 0;

        virtual bool isIntraprocess() const { return false; }
        virtual void getPublishTypes(bool& ser, bool& nocopy, const std::type_info& ti);

        const std::string& getMD5Sum() const;
        const std::string& getDataType() const;
        const std::string& getMessageDefinition() const;

    protected:
        bool verifyDatatype(const std::string& datatype) const;

        RosNodePtr node_;

        PublicationWPtr parent_;
        unsigned int connection_id_;
        std::string destination_caller_id_;
        Stats stats_;
        std::string topic_;
};

} // namespace roscan

#endif // ROSCAN_SUBSCRIBER_LINK_H
