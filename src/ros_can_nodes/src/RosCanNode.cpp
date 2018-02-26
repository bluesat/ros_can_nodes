#include "common.h"
#include "RosCanNode.h"
#include <iostream>
//#include "subscriber.h"
//#include <ros/subscribe_options.h>
#include <unistd.h>

namespace roscan {

RosCanNode::RosCanNode(std::string name) : name_(name) {
    RosCanNodePtr nodeptr = boost::make_shared<RosCanNode>(*this);

    std::cout << "  Creating xmlrpc manager\n";
    xmlrpcManager.reset(new XMLRPCManager);

    std::cout << "  Creating poll manager\n";
    pollManager.reset(new PollManager);
    std::cout << "  Starting poll manager\n";
    pollManager->start();

    std::cout << "  Creating connection manager\n";
    connectionManager.reset(new ConnectionManager);
    std::cout << "  Starting connection manager\n";
    connectionManager->start(nodeptr);

    std::cout << "  Creating topic manager\n";
    topicManager.reset(new TopicManager);
    std::cout << "  Starting topic manager\n";
    topicManager->start(nodeptr);
    std::cout << "  Done!\n";

    // xmlrpc manager must be started _after_ all functions are bound to it
    std::cout << "  Starting xmlrpc manager\n";
    xmlrpcManager->start();

    //collection_ = new NodeBackingCollection;
    //callback_queue_ = new ros::CallbackQueue;
}

/*
void RosCanNode::subChatterCallback(const boost::shared_ptr<std_msgs::String const>& msg) {
    std::cout << "received " << msg->data << std::endl;
}

Subscriber RosCanNode::subscribe(ros::SubscribeOptions& ops) {
    if (ops.callback_queue == 0) {
        if (callback_queue_) {
            ops.callback_queue = callback_queue_;
        } else {
            std::cout << "argh no callback queue\n";
            return Subscriber();
        }
    }

    if (topicManager->subscribe(ops)) {
        Subscriber sub(ops.topic, boost::make_shared<RosCanNode>(*this), ops.helper);

        {
            boost::mutex::scoped_lock lock(collection_->mutex_);
            collection_->subs_.push_back(sub);
        }

        return sub;
    }

    return Subscriber();
}

void RosCanNode::spinOnce() {
    ((ros::CallbackQueue*)callback_queue_)->callAvailable(ros::WallDuration());
}
*/

} // namespace roscan