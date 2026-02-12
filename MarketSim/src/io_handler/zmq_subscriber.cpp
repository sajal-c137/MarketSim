#include "zmq_subscriber.h"

namespace marketsim::io_handler {

ZmqSubscriber::ZmqSubscriber(IOContext& context, const std::string& name, const std::string& endpoint)
    : socket_(context.get_context(), zmq::socket_type::sub)
    , endpoint_(endpoint)
    , connected_(false)
    , monitor_(std::make_unique<monitor::MonitoredSocket>(
        name,
        monitor::SocketType::SUB,
        endpoint
    ))
{
    monitor_->update_state(monitor::SocketState::DISCONNECTED);
}

ZmqSubscriber::~ZmqSubscriber() {
    close();
}

void ZmqSubscriber::connect() {
    try {
        socket_.connect(endpoint_);
        connected_ = true;
        monitor_->update_state(monitor::SocketState::CONNECTED);
    } catch (const zmq::error_t& e) {
        monitor_->record_error(std::string("Connect failed: ") + e.what());
        monitor_->update_state(monitor::SocketState::ERROR);
        throw;
    }
}

void ZmqSubscriber::subscribe(const std::string& topic) {
    try {
        socket_.set(zmq::sockopt::subscribe, topic);
    } catch (const zmq::error_t& e) {
        monitor_->record_error(std::string("Subscribe failed: ") + e.what());
        throw;
    }
}

void ZmqSubscriber::unsubscribe(const std::string& topic) {
    try {
        socket_.set(zmq::sockopt::unsubscribe, topic);
    } catch (const zmq::error_t& e) {
        monitor_->record_error(std::string("Unsubscribe failed: ") + e.what());
        throw;
    }
}

bool ZmqSubscriber::receive(google::protobuf::Message& message) {
    if (!connected_) {
        monitor_->record_error("Cannot receive: socket not connected");
        return false;
    }
    
    try {
        zmq::message_t zmq_msg;
        auto result = socket_.recv(zmq_msg, zmq::recv_flags::none);
        
        if (result) {
            std::string data(static_cast<char*>(zmq_msg.data()), zmq_msg.size());
            if (MessageSerializer::deserialize(data, message)) {
                monitor_->record_receive(zmq_msg.size());
                return true;
            } else {
                monitor_->record_error("Deserialization failed");
                return false;
            }
        } else {
            monitor_->record_error("Receive failed: no result");
            return false;
        }
    } catch (const zmq::error_t& e) {
        monitor_->record_error(std::string("Receive failed: ") + e.what());
        monitor_->update_state(monitor::SocketState::ERROR);
        return false;
    }
}

bool ZmqSubscriber::receive_with_timeout(google::protobuf::Message& message, int timeout_ms) {
    if (!connected_) {
        monitor_->record_error("Cannot receive: socket not connected");
        return false;
    }
    
    try {
        socket_.set(zmq::sockopt::rcvtimeo, timeout_ms);
        
        zmq::message_t zmq_msg;
        auto result = socket_.recv(zmq_msg, zmq::recv_flags::none);
        
        socket_.set(zmq::sockopt::rcvtimeo, -1);
        
        if (result) {
            std::string data(static_cast<char*>(zmq_msg.data()), zmq_msg.size());
            if (MessageSerializer::deserialize(data, message)) {
                monitor_->record_receive(zmq_msg.size());
                return true;
            } else {
                monitor_->record_error("Deserialization failed");
                return false;
            }
        } else {
            return false;
        }
    } catch (const zmq::error_t& e) {
        if (e.num() == EAGAIN) {
            return false;
        }
        monitor_->record_error(std::string("Receive failed: ") + e.what());
        monitor_->update_state(monitor::SocketState::ERROR);
        return false;
    }
}

bool ZmqSubscriber::receive_with_topic(std::string& topic, google::protobuf::Message& message) {
    if (!connected_) {
        monitor_->record_error("Cannot receive: socket not connected");
        return false;
    }
    
    try {
        zmq::message_t topic_msg;
        auto result = socket_.recv(topic_msg, zmq::recv_flags::none);
        
        if (!result) {
            monitor_->record_error("Failed to receive topic frame");
            return false;
        }
        
        topic = std::string(static_cast<char*>(topic_msg.data()), topic_msg.size());
        
        zmq::message_t data_msg;
        result = socket_.recv(data_msg, zmq::recv_flags::none);
        
        if (result) {
            std::string data(static_cast<char*>(data_msg.data()), data_msg.size());
            if (MessageSerializer::deserialize(data, message)) {
                monitor_->record_receive(topic_msg.size() + data_msg.size());
                return true;
            } else {
                monitor_->record_error("Deserialization failed");
                return false;
            }
        } else {
            monitor_->record_error("Failed to receive data frame");
            return false;
        }
    } catch (const zmq::error_t& e) {
        monitor_->record_error(std::string("Receive with topic failed: ") + e.what());
        monitor_->update_state(monitor::SocketState::ERROR);
        return false;
    }
}

void ZmqSubscriber::close() {
    if (connected_) {
        socket_.close();
        connected_ = false;
        monitor_->update_state(monitor::SocketState::DISCONNECTED);
    }
}

bool ZmqSubscriber::is_connected() const {
    return connected_;
}

}
