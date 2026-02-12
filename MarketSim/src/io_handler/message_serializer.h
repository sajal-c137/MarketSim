#pragma once

#include <google/protobuf/message.h>
#include <string>
#include <vector>
#include <optional>

namespace marketsim::io_handler {

/**
 * @brief Handles Protobuf message serialization and deserialization
 */
class MessageSerializer {
public:
    /**
     * @brief Serialize a protobuf message to binary string
     * @param message The protobuf message to serialize
     * @return Serialized binary string
     * @throws std::runtime_error if serialization fails
     */
    static std::string serialize(const google::protobuf::Message& message);
    
    /**
     * @brief Deserialize binary data into a protobuf message
     * @param data Binary data to deserialize
     * @param message Output message to populate
     * @return true if successful, false otherwise
     */
    static bool deserialize(const std::string& data, google::protobuf::Message& message);
    
    /**
     * @brief Serialize to a vector of bytes (for ZMQ compatibility)
     * @param message The protobuf message to serialize
     * @return Vector of bytes
     */
    static std::vector<uint8_t> serialize_to_bytes(const google::protobuf::Message& message);
    
    /**
     * @brief Deserialize from a vector of bytes
     * @param data Vector of bytes
     * @param message Output message to populate
     * @return true if successful, false otherwise
     */
    static bool deserialize_from_bytes(const std::vector<uint8_t>& data, google::protobuf::Message& message);
};

}
