#include "zmq_publisher.h"
#include <iostream>

namespace marketsim::io_handler {

ZmqPublisher::ZmqPublisher(IOContext& context, const std::string& name, const std::string& endpoint)
    : socket_(context.get_context(), zmq::socket_type::pub)
    , endpoint_(endpoint)
    , bound_(false)
    , monitor_(std::make_unique<monitor::MonitoredSocket>(
        name, 
        monitor::SocketType::PUB, 
        endpoint
    ))
{
    monitor_->update_state(monitor::SocketState::DISCONNECTED);
}

ZmqPublisher::~ZmqPublisher() {
    close();
}

void ZmqPublisher::bind() {
    try {
        socket_.bind(endpoint_);
        bound_ = true;
        monitor_->update_state(monitor::SocketState::LISTENING);
    } catch (const zmq::error_t& e) {
        monitor_->record_error(std::string("Bind failed: ") + e.what());
        monitor_->update_state(monitor::SocketState::ERROR);
        throw;
    }
}

bool ZmqPublisher::publish(const google::protobuf::Message& message) {
    if (!bound_) {
        monitor_->record_error("Cannot publish: socket not bound");
        return false;
    }
    
    try {
        std::string serialized = MessageSerializer::serialize(message);
        zmq::message_t zmq_msg(serialized.data(), serialized.size());
        
        auto result = socket_.send(zmq_msg, zmq::send_flags::none);
        if (result) {
            monitor_->record_send(serialized.size());
            return true;
        } else {
            monitor_->record_error("Send failed: no result");
            return false;
        }
    } catch (const zmq::error_t& e) {
        monitor_->record_error(std::string("Send failed: ") + e.what());
        monitor_->update_state(monitor::SocketState::ERROR);
        return false;
    } catch (const std::exception& e) {
        monitor_->record_error(std::string("Serialization failed: ") + e.what());
        return false;
    }
}

bool ZmqPublisher::publish_with_topic(const std::string& topic, const google::protobuf::Message& message) {
    if (!bound_) {
        monitor_->record_error("Cannot publish: socket not bound");
        return false;
    }
    
    try {
        // Send topic as first frame
        zmq::message_t topic_msg(topic.data(), topic.size());
        auto result = socket_.send(topic_msg, zmq::send_flags::sndmore);
        
        if (!result) {
            monitor_->record_error("Failed to send topic frame");
            return false;
        }
        
        // Send message as second frame
        std::string serialized = MessageSerializer::serialize(message);
        zmq::message_t data_msg(serialized.data(), serialized.size());
        
        result = socket_.send(data_msg, zmq::send_flags::none);
        if (result) {
            monitor_->record_send(topic.size() + serialized.size());
            return true;
        } else {
            monitor_->record_error("Failed to send data frame");
            return false;
        }
    } catch (const zmq::error_t& e) {
        monitor_->record_error(std::string("Publish with topic failed: ") + e.what());
        monitor_->update_state(monitor::SocketState::ERROR);
        return false;
    } catch (const std::exception& e) {
        monitor_->record_error(std::string("Serialization failed: ") + e.what());
        return false;
    }
}

void ZmqPublisher::close() {
    if (bound_) {
        socket_.close();
        bound_ = false;
        monitor_->update_state(monitor::SocketState::DISCONNECTED);
    }
}

bool ZmqPublisher::is_connected() const {
    return bound_;
}

}
