#include "zmq_replier.h"

namespace marketsim::io_handler {

ZmqReplier::ZmqReplier(IOContext& context, const std::string& name, const std::string& endpoint)
    : socket_(context.get_context(), zmq::socket_type::rep)
    , endpoint_(endpoint)
    , bound_(false)
    , waiting_for_response_(false)
    , monitor_(std::make_unique<monitor::MonitoredSocket>(
        name,
        monitor::SocketType::REP,
        endpoint
    ))
{
    monitor_->update_state(monitor::SocketState::DISCONNECTED);
}

ZmqReplier::~ZmqReplier() {
    close();
}

void ZmqReplier::bind() {
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

bool ZmqReplier::reply(google::protobuf::Message& request, const google::protobuf::Message& response) {
    if (!bound_) {
        monitor_->record_error("Cannot reply: socket not bound");
        return false;
    }
    
    try {
        zmq::message_t req_msg;
        auto recv_result = socket_.recv(req_msg, zmq::recv_flags::none);
        
        if (!recv_result) {
            monitor_->record_error("Failed to receive request");
            return false;
        }
        
        std::string req_data(static_cast<char*>(req_msg.data()), req_msg.size());
        if (!MessageSerializer::deserialize(req_data, request)) {
            monitor_->record_error("Request deserialization failed");
            return false;
        }
        monitor_->record_receive(req_msg.size());
        
        std::string serialized_resp = MessageSerializer::serialize(response);
        zmq::message_t resp_msg(serialized_resp.data(), serialized_resp.size());
        
        auto send_result = socket_.send(resp_msg, zmq::send_flags::none);
        if (send_result) {
            monitor_->record_send(serialized_resp.size());
            return true;
        } else {
            monitor_->record_error("Failed to send response");
            return false;
        }
    } catch (const zmq::error_t& e) {
        monitor_->record_error(std::string("Reply failed: ") + e.what());
        monitor_->update_state(monitor::SocketState::ERROR);
        return false;
    } catch (const std::exception& e) {
        monitor_->record_error(std::string("Reply failed: ") + e.what());
        return false;
    }
}

bool ZmqReplier::receive_request(google::protobuf::Message& request, int timeout_ms) {
    if (!bound_) {
        monitor_->record_error("Cannot receive: socket not bound");
        return false;
    }
    
    if (waiting_for_response_) {
        monitor_->record_error("Cannot receive: waiting for response to previous request");
        return false;
    }
    
    try {
        if (timeout_ms >= 0) {
            socket_.set(zmq::sockopt::rcvtimeo, timeout_ms);
        }
        
        zmq::message_t req_msg;
        auto recv_result = socket_.recv(req_msg, zmq::recv_flags::none);
        
        if (timeout_ms >= 0) {
            socket_.set(zmq::sockopt::rcvtimeo, -1);
        }
        
        if (recv_result) {
            std::string req_data(static_cast<char*>(req_msg.data()), req_msg.size());
            if (MessageSerializer::deserialize(req_data, request)) {
                monitor_->record_receive(req_msg.size());
                waiting_for_response_ = true;
                return true;
            } else {
                monitor_->record_error("Request deserialization failed");
                return false;
            }
        } else {
            return false;
        }
    } catch (const zmq::error_t& e) {
        if (e.num() == EAGAIN) {
            return false;
        }
        monitor_->record_error(std::string("Receive request failed: ") + e.what());
        monitor_->update_state(monitor::SocketState::ERROR);
        return false;
    }
}

bool ZmqReplier::send_response(const google::protobuf::Message& response) {
    if (!bound_) {
        monitor_->record_error("Cannot send response: socket not bound");
        return false;
    }
    
    if (!waiting_for_response_) {
        monitor_->record_error("Cannot send response: no pending request");
        return false;
    }
    
    try {
        std::string serialized_resp = MessageSerializer::serialize(response);
        zmq::message_t resp_msg(serialized_resp.data(), serialized_resp.size());
        
        auto send_result = socket_.send(resp_msg, zmq::send_flags::none);
        if (send_result) {
            monitor_->record_send(serialized_resp.size());
            waiting_for_response_ = false;
            return true;
        } else {
            monitor_->record_error("Failed to send response");
            return false;
        }
    } catch (const zmq::error_t& e) {
        monitor_->record_error(std::string("Send response failed: ") + e.what());
        monitor_->update_state(monitor::SocketState::ERROR);
        return false;
    } catch (const std::exception& e) {
        monitor_->record_error(std::string("Send response failed: ") + e.what());
        return false;
    }
}

void ZmqReplier::close() {
    if (bound_) {
        socket_.close();
        bound_ = false;
        waiting_for_response_ = false;
        monitor_->update_state(monitor::SocketState::DISCONNECTED);
    }
}

bool ZmqReplier::is_bound() const {
    return bound_;
}

}
