#pragma once

#include "io_context.h"
#include "message_serializer.h"
#include "monitor/monitor_helpers.h"
#include "monitor/socket_info.h"
#include <zmq.hpp>
#include <string>
#include <memory>
#include <optional>
#include <google/protobuf/message.h>

namespace marketsim::io_handler {

/**
 * @brief ZeroMQ Subscriber (SUB socket) with monitoring
 * 
 * Used for receiving market data broadcasts.
 * Each instance is owned by a single thread pool.
 */
class ZmqSubscriber {
public:
    /**
     * @brief Construct a subscriber
     * @param context IOContext for this component
     * @param name Unique name for monitoring
     * @param endpoint Endpoint to connect (e.g., "tcp://localhost:5555")
     */
    ZmqSubscriber(IOContext& context, const std::string& name, const std::string& endpoint);
    
    ~ZmqSubscriber();
    
    /**
     * @brief Connect to the publisher
     * @throws zmq::error_t on failure
     */
    void connect();
    
    /**
     * @brief Subscribe to a specific topic filter
     * @param topic Topic to subscribe to ("" for all messages)
     */
    void subscribe(const std::string& topic = "");
    
    /**
     * @brief Unsubscribe from a topic filter
     * @param topic Topic to unsubscribe from
     */
    void unsubscribe(const std::string& topic);
    
    /**
     * @brief Receive a message (blocking)
     * @param message Output message to populate
     * @return true if message received, false on error
     */
    bool receive(google::protobuf::Message& message);
    
    /**
     * @brief Receive a message with timeout
     * @param message Output message to populate
     * @param timeout_ms Timeout in milliseconds
     * @return true if message received, false on timeout or error
     */
    bool receive_with_timeout(google::protobuf::Message& message, int timeout_ms);
    
    /**
     * @brief Receive a message with topic
     * @param topic Output topic string
     * @param message Output message to populate
     * @return true if message received, false on error
     */
    bool receive_with_topic(std::string& topic, google::protobuf::Message& message);
    
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
