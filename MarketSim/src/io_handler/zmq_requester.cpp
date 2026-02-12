#include "zmq_requester.h"

namespace marketsim::io_handler {

ZmqRequester::ZmqRequester(IOContext& context, const std::string& name, const std::string& endpoint)
    : socket_(context.get_context(), zmq::socket_type::req)
    , endpoint_(endpoint)
    , connected_(false)
    , monitor_(std::make_unique<monitor::MonitoredSocket>(
        name,
        monitor::SocketType::REQ,
        endpoint
    ))
{
    monitor_->update_state(monitor::SocketState::DISCONNECTED);
}

ZmqRequester::~ZmqRequester() {
    close();
}

void ZmqRequester::connect() {
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

bool ZmqRequester::request(const google::protobuf::Message& request, google::protobuf::Message& response) {
    if (!connected_) {
        monitor_->record_error("Cannot send request: socket not connected");
        return false;
    }
    
    try {
        std::string serialized_req = MessageSerializer::serialize(request);
        zmq::message_t req_msg(serialized_req.data(), serialized_req.size());
        
        auto send_result = socket_.send(req_msg, zmq::send_flags::none);
        if (!send_result) {
            monitor_->record_error("Failed to send request");
            return false;
        }
        monitor_->record_send(serialized_req.size());
        
        zmq::message_t resp_msg;
        auto recv_result = socket_.recv(resp_msg, zmq::recv_flags::none);
        
        if (recv_result) {
            std::string data(static_cast<char*>(resp_msg.data()), resp_msg.size());
            if (MessageSerializer::deserialize(data, response)) {
                monitor_->record_receive(resp_msg.size());
                return true;
            } else {
                monitor_->record_error("Response deserialization failed");
                return false;
            }
        } else {
            monitor_->record_error("Failed to receive response");
            return false;
        }
    } catch (const zmq::error_t& e) {
        monitor_->record_error(std::string("Request failed: ") + e.what());
        monitor_->update_state(monitor::SocketState::ERROR);
        return false;
    } catch (const std::exception& e) {
        monitor_->record_error(std::string("Request failed: ") + e.what());
        return false;
    }
}

bool ZmqRequester::request_with_timeout(const google::protobuf::Message& request,
                                        google::protobuf::Message& response,
                                        int timeout_ms) {
    if (!connected_) {
        monitor_->record_error("Cannot send request: socket not connected");
        return false;
    }
    
    try {
        socket_.set(zmq::sockopt::rcvtimeo, timeout_ms);
        
        std::string serialized_req = MessageSerializer::serialize(request);
        zmq::message_t req_msg(serialized_req.data(), serialized_req.size());
        
        auto send_result = socket_.send(req_msg, zmq::send_flags::none);
        if (!send_result) {
            monitor_->record_error("Failed to send request");
            socket_.set(zmq::sockopt::rcvtimeo, -1);
            return false;
        }
        monitor_->record_send(serialized_req.size());
        
        zmq::message_t resp_msg;
        auto recv_result = socket_.recv(resp_msg, zmq::recv_flags::none);
        
        socket_.set(zmq::sockopt::rcvtimeo, -1);
        
        if (recv_result) {
            std::string data(static_cast<char*>(resp_msg.data()), resp_msg.size());
            if (MessageSerializer::deserialize(data, response)) {
                monitor_->record_receive(resp_msg.size());
                return true;
            } else {
                monitor_->record_error("Response deserialization failed");
                return false;
            }
        } else {
            return false;
        }
    } catch (const zmq::error_t& e) {
        if (e.num() == EAGAIN) {
            return false;
        }
        monitor_->record_error(std::string("Request with timeout failed: ") + e.what());
        monitor_->update_state(monitor::SocketState::ERROR);
        return false;
    } catch (const std::exception& e) {
        monitor_->record_error(std::string("Request failed: ") + e.what());
        return false;
    }
}

void ZmqRequester::close() {
    if (connected_) {
        socket_.close();
        connected_ = false;
        monitor_->update_state(monitor::SocketState::DISCONNECTED);
    }
}

bool ZmqRequester::is_connected() const {
    return connected_;
}

}
