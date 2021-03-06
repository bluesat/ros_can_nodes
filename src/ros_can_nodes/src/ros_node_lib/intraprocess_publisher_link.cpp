/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2008, Willow Garage, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Willow Garage, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#include "ros_node_lib/RosNode.hpp"
#include "ros_node_lib/subscription.h"
#include "ros_node_lib/intraprocess_publisher_link.h"
#include "ros_node_lib/intraprocess_subscriber_link.h"
#include <ros/connection.h>
#include <ros/header.h>
#include <ros/transport/transport.h>

namespace roscan {

void IntraProcessPublisherLink::setPublisher(const IntraProcessSubscriberLinkPtr& publisher) {
    publisher_ = publisher;

    const auto parent = parent_.lock();

    ros::Header header;
    ros::M_stringPtr values = header.getValues();
    (*values)["callerid"] = node_->getName();
    (*values)["topic"] = parent->getName();
    (*values)["type"] = publisher->getDataType();
    (*values)["md5sum"] = publisher->getMD5Sum();
    (*values)["message_definition"] = publisher->getMessageDefinition();
    (*values)["latching"] = publisher->isLatching() ? "1" : "0";
    setHeader(header);
}

void IntraProcessPublisherLink::drop() {
    {
        std::lock_guard<std::recursive_mutex> lock{drop_mutex_};
        if (dropped_) {
            return;
        }
        dropped_ = true;
    }

    if (publisher_) {
        publisher_->drop();
        publisher_.reset();
    }

    if (const auto parent = parent_.lock()) {
        parent->removePublisherLink(shared_from_this());
    }
}

void IntraProcessPublisherLink::handleMessage(const ros::SerializedMessage& m, bool ser, bool nocopy) {
    std::lock_guard<std::recursive_mutex> lock{drop_mutex_};
    if (dropped_) {
        return;
    }

    stats_.bytes_received_ += m.num_bytes;
    ++stats_.messages_received_;

    if (const auto parent = parent_.lock()) {
        stats_.drops_ += parent->handleMessage(m, ser, nocopy, header_.getValues(), shared_from_this());
    }
}

void IntraProcessPublisherLink::getPublishTypes(bool& ser, bool& nocopy, const std::type_info& ti) {
    std::lock_guard<std::recursive_mutex> lock{drop_mutex_};
    if (dropped_) {
        ser = false;
        nocopy = false;
        return;
    }

    if (const auto parent = parent_.lock()) {
        parent->getPublishTypes(ser, nocopy, ti);
    } else {
        ser = true;
        nocopy = false;
    }
}

} // namespace roscan
