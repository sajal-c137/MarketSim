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
 * @brief ZeroMQ Publisher (PUB socket) with monitoring
 * 
 * Used for broadcasting market data to multiple subscribers.
 * Each instance is owned by a single thread pool.
 */
class ZmqPublisher {
public:
    /**
     * @brief Construct a publisher
     * @param context IOContext for this component
     * @param name Unique name for monitoring
     * @param endpoint Endpoint to bind (e.g., "tcp://*:5555")
     */
    ZmqPublisher(IOContext& context, const std::string& name, const std::string& endpoint);
    
    ~ZmqPublisher();
    
    /**
     * @brief Bind the socket to the endpoint
     * @throws zmq::error_t on failure
     */
    void bind();
    
    /**
     * @brief Publish a protobuf message
     * @param message The protobuf message to publish
     * @return true if successful, false otherwise
     */
    bool publish(const google::protobuf::Message& message);
    
    /**
     * @brief Publish a protobuf message with a topic prefix
     * @param topic Topic string for filtering (e.g., "AAPL", "BTC")
     * @param message The protobuf message to publish
     * @return true if successful, false otherwise
     */
    bool publish_with_topic(const std::string& topic, const google::protobuf::Message& message);
    
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
    bool bound_;
    std::unique_ptr<monitor::MonitoredSocket> monitor_;
};

}
