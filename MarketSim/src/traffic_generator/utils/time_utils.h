#pragma once

#include <cstdint>

namespace marketsim::traffic_generator::utils {

/**
 * @brief Time utilities for traffic generation
 */
class TimeUtils {
public:
    /**
     * @brief Get current timestamp in milliseconds since epoch
     */
    static int64_t current_timestamp_ms();
    
    /**
     * @brief Get current timestamp in microseconds since epoch
     */
    static int64_t current_timestamp_us();
    
    /**
     * @brief Calculate elapsed time in seconds
     * @param start_ms Start timestamp in milliseconds
     * @return Elapsed seconds as double
     */
    static double elapsed_seconds(int64_t start_ms);
    
    /**
     * @brief Sleep for specified milliseconds
     */
    static void sleep_ms(int64_t milliseconds);
};

} // namespace marketsim::traffic_generator::utils
