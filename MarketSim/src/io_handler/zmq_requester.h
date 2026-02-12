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
 * @brief ZeroMQ Requester (REQ socket) with monitoring
 * 
 * Used for sending requests and receiving responses (client side).
 * Each instance is owned by a single thread pool.
 */
class ZmqRequester {
public:
    /**
     * @brief Construct a requester
     * @param context IOContext for this component
     * @param name Unique name for monitoring
     * @param endpoint Endpoint to connect (e.g., "tcp://localhost:5556")
     */
    ZmqRequester(IOContext& context, const std::string& name, const std::string& endpoint);
    
    ~ZmqRequester();
    
    /**
     * @brief Connect to the replier
     * @throws zmq::error_t on failure
     */
    void connect();
    
    /**
     * @brief Send a request and wait for response (blocking)
     * @param request The request message
     * @param response The response message to populate
     * @return true if successful, false on error
     */
    bool request(const google::protobuf::Message& request, google::protobuf::Message& response);
    
    /**
     * @brief Send a request and wait for response with timeout
     * @param request The request message
     * @param response The response message to populate
     * @param timeout_ms Timeout in milliseconds
     * @return true if successful, false on timeout or error
     */
    bool request_with_timeout(const google::protobuf::Message& request, 
                              google::protobuf::Message& response, 
                              int timeout_ms);
    
    /**
     * @brief Close the socket
     */
    void close();
    
    /**
     * @brief Check if socket is connected
     */
    bool is_connected() const;
    
private:
    zmq::socket_t socket_;
    std::string endpoint_;
    bool connected_;
    std::unique_ptr<monitor::MonitoredSocket> monitor_;
};

}
