#include "common.h"
#include "RosCanNode.h"
#include "rosout_appender.h"
#include "advertise_options.h"
#include "callback_queue.h"
#include "publisher.h"
#include "subscriber.h"
#include "internal_timer_manager.h"
#include <iostream>
#include <ros/console.h>
#include <ros/file_log.h>
#include <ros/transport/transport_tcp.h>
#include <xmlrpcpp/XmlRpcSocket.h>
#include <unistd.h>

namespace roscan {

class NodeBackingCollection {
    public:
        //typedef std::vector<Publisher::ImplWPtr> V_PubImpl;
        //typedef std::vector<ServiceServer::ImplWPtr> V_SrvImpl;
        //typedef std::vector<Subscriber::ImplWPtr> V_SubImpl;
        //typedef std::vector<ServiceClient::ImplWPtr> V_SrvCImpl;

        typedef std::vector<PublisherPtr> V_Pubs;
        //typedef std::vector<ServiceServer::ImplWPtr> V_SrvImpl;
        typedef std::vector<SubscriberPtr> V_Subs;
        //typedef std::vector<ServiceClient::ImplWPtr> V_SrvCImpl;
        V_Pubs pubs_;
        //V_SrvImpl srvs_;
        V_Subs subs_;
        //V_SrvCImpl srv_cs_;

        boost::mutex mutex_;
};

void RosCanNode::check_ipv6_environment() {
    char* env_ipv6 = NULL;
    env_ipv6 = getenv("ROS_IPV6");

    bool use_ipv6 = (env_ipv6 && strcmp(env_ipv6, "on") == 0);
    ros::TransportTCP::s_use_ipv6_ = use_ipv6;
    XmlRpc::XmlRpcSocket::s_use_ipv6_ = use_ipv6;
}

RosCanNode::RosCanNode(std::string name) : name_("/" + name), g_started(false), g_shutting_down(false), callback_queue_(0), collection_(0) {
    std::cout << "Creating node " << name_ << "\n";
    g_global_queue.reset(new CallbackQueue);
    ROSCONSOLE_AUTOINIT;
    check_ipv6_environment();
    collection_ = new NodeBackingCollection;
    std::cout << "Created node " << name_ << "\n";
}

RosCanNode::~RosCanNode() {
    shutdown();
    std::cout << "Deleting node " << name_ << "\n";
    delete collection_;
    std::cout << "Deleted node " << name_ << "\n";
}

CallbackQueuePtr RosCanNode::getInternalCallbackQueue() {
    if (!g_internal_callback_queue) {
        g_internal_callback_queue.reset(new CallbackQueue);
    }
    return g_internal_callback_queue;
}

void RosCanNode::getAdvertisedTopics(V_string& topics) {
    topic_manager()->getAdvertisedTopics(topics);
}

void RosCanNode::getSubscribedTopics(V_string& topics) {
    topic_manager()->getSubscribedTopics(topics);
}

const TopicManagerPtr& RosCanNode::topic_manager() {
    if (!topicManager) {
        topicManager.reset(new TopicManager(shared_from_this()));
    }
    return topicManager;
}

const ConnectionManagerPtr& RosCanNode::connection_manager() {
    if (!connectionManager) {
        connectionManager.reset(new ConnectionManager(shared_from_this()));
    }
    return connectionManager;
}

const PollManagerPtr& RosCanNode::poll_manager() {
    if (!pollManager) {
        pollManager.reset(new PollManager());
    }
    return pollManager;
}

const XMLRPCManagerPtr& RosCanNode::xmlrpc_manager() {
    if (!xmlrpcManager) {
        xmlrpcManager.reset(new XMLRPCManager());
    }
    return xmlrpcManager;
}

void RosCanNode::internalCallbackQueueThreadFunc() {
    ros::disableAllSignalsInThisThread();

    CallbackQueuePtr queue = getInternalCallbackQueue();

    while (!g_shutting_down) {
        queue->callAvailable(ros::WallDuration(0.1));
    }
}

void RosCanNode::start() {
    std::cout << "Starting node " << name_ << "\n";
    initInternalTimerManager();

    poll_manager()->start();
    connection_manager()->start();
    topic_manager()->start();
    // xmlrpc manager must be started _after_ all functions are bound to it
    xmlrpc_manager()->start();

    ros::Time::init();

    g_rosout_appender = new ROSOutAppender(shared_from_this());
    ros::console::register_appender(g_rosout_appender);

    g_internal_queue_thread = boost::thread(&RosCanNode::internalCallbackQueueThreadFunc, this);
    getGlobalCallbackQueue()->enable();
    g_started = true;
    std::cout << "Started node " << name_ << "\n";
}

void RosCanNode::shutdown() {
    if (g_shutting_down) {
        return;
    }
    std::cout << "Shutting down node " << name_ << "\n";
    g_shutting_down = true;
    //ros::console::shutdown();

    g_global_queue->disable();
    g_global_queue->clear();

    if (g_internal_queue_thread.get_id() != boost::this_thread::get_id()) {
        g_internal_queue_thread.join();
    }

    delete collection_;

    g_rosout_appender = 0;

    if (g_started) {
        topic_manager()->shutdown();
        poll_manager()->shutdown();
        connection_manager()->shutdown();
        xmlrpc_manager()->shutdown();
    }
    //ros::Time::shutdown();
    g_started = false;
    std::cout << "Shut down node " << name_ << "\n";
}

PublisherPtr RosCanNode::advertise(AdvertiseOptions& ops) {
    if (ops.callback_queue == 0) {
        if (callback_queue_) {
            ops.callback_queue = callback_queue_;
        } else {
            ops.callback_queue = getGlobalCallbackQueue();
        }
    }

    SubscriberCallbacksPtr callbacks(boost::make_shared<SubscriberCallbacks>(ops.connect_cb, ops.disconnect_cb, ops.tracked_object, ops.callback_queue));

    if (topic_manager()->advertise(ops, callbacks)) {
        PublisherPtr pub = boost::make_shared<Publisher>(ops.topic, shared_from_this(), ops.md5sum, ops.datatype, callbacks);

        {
            boost::mutex::scoped_lock lock(collection_->mutex_);
            collection_->pubs_.push_back(pub);
        }
        return pub;
    }
    return boost::make_shared<Publisher>();
}

SubscriberPtr RosCanNode::subscribe(SubscribeOptions& ops) {
    if (ops.callback_queue == 0) {
        if (callback_queue_) {
            ops.callback_queue = callback_queue_;
        } else {
            ops.callback_queue = getGlobalCallbackQueue();
        }
    }

    if (topicManager->subscribe(ops)) {
        SubscriberPtr sub = boost::make_shared<Subscriber>(ops.topic, shared_from_this(), ops.helper);

        {
            boost::mutex::scoped_lock lock(collection_->mutex_);
            collection_->subs_.push_back(sub);
        }
        return sub;
    }
    return boost::make_shared<Subscriber>();
}

void RosCanNode::spinOnce() {
    g_global_queue->callAvailable(ros::WallDuration());
}

} // namespace roscan
