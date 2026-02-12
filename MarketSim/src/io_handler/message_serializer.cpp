#include "message_serializer.h"
#include <stdexcept>

namespace marketsim::io_handler {

std::string MessageSerializer::serialize(const google::protobuf::Message& message) {
    std::string serialized;
    if (!message.SerializeToString(&serialized)) {
        std::string error_msg = "Failed to serialize protobuf message: ";
        error_msg += std::string(message.GetTypeName());
        throw std::runtime_error(error_msg);
    }
    return serialized;
}

bool MessageSerializer::deserialize(const std::string& data, google::protobuf::Message& message) {
    return message.ParseFromString(data);
}

std::vector<uint8_t> MessageSerializer::serialize_to_bytes(const google::protobuf::Message& message) {
    std::string serialized = serialize(message);
    return std::vector<uint8_t>(serialized.begin(), serialized.end());
}

bool MessageSerializer::deserialize_from_bytes(const std::vector<uint8_t>& data, google::protobuf::Message& message) {
    std::string str(data.begin(), data.end());
    return deserialize(str, message);
}

}
