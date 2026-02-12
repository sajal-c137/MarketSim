#pragma once

#include "io_context.h"
#include "message_serializer.h"
#include "monitor/monitor_helpers.h"
#include "monitor/socket_info.h"
#include <zmq.hpp>
#include <string>
#include <memory>
#include <google/protobuf/message.h>

namespace marketsim::io_handler {

/**
 * @brief ZeroMQ Replier (REP socket) with monitoring
 * 
 * Used for receiving requests and sending responses (server side).
 * Each instance is owned by a single thread pool.
 */
class ZmqReplier {
public:
    /**
     * @brief Construct a replier
     * @param context IOContext for this component
     * @param name Unique name for monitoring
     * @param endpoint Endpoint to bind (e.g., "tcp://*:5556")
     */
    ZmqReplier(IOContext& context, const std::string& name, const std::string& endpoint);
    
    ~ZmqReplier();
    
    /**
     * @brief Bind the socket to the endpoint
     * @throws zmq::error_t on failure
     */
    void bind();
    
    /**
     * @brief Receive a request and send a response (blocking)
     * @param request The request message to populate
     * @param response The response message to send
     * @return true if successful, false on error
     */
    bool reply(google::protobuf::Message& request, const google::protobuf::Message& response);
    
    /**
     * @brief Receive a request with timeout
     * @param request The request message to populate
     * @param timeout_ms Timeout in milliseconds
     * @return true if request received, false on timeout or error
     */
    bool receive_request(google::protobuf::Message& request, int timeout_ms = -1);
    
    /**
     * @brief Send a response (must be called after receive_request)
     * @param response The response message to send
     * @return true if successful, false on error
     */
    bool send_response(const google::protobuf::Message& response);
    
    /**
     * @brief Close the socket
     */
    void close();
    
    /**
     * @brief Check if socket is bound
     */
    bool is_bound() const;
    
private:
    zmq::socket_t socket_;
    std::string endpoint_;
    bool bound_;
    bool waiting_for_response_;
    std::unique_ptr<monitor::MonitoredSocket> monitor_;
};

}
