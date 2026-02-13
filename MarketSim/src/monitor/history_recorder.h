#pragma once

#include "monitor_config.h"
#include "exchange.pb.h"
#include <string>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace marketsim::monitor {

/**
 * @brief Records market data history to files for post-session analysis
 * 
 * Creates CSV files with historical data:
 * - Trade prices over time
 * - Mid prices over time
 * - Orderbook snapshots (optional)
 */
class HistoryRecorder {
public:
    explicit HistoryRecorder(const HistoryRecorderConfig& config);
    ~HistoryRecorder();
    
    /**
     * @brief Start recording session
     * @param symbol Trading symbol (used in filename)
     */
    void start_session(const std::string& symbol);
    
    /**
     * @brief Record a status update from Exchange
     */
    void record_status(const marketsim::exchange::StatusResponse& response);
    
    /**
     * @brief End recording session and close files
     */
    void end_session();
    
    /**
     * @brief Check if recording is active
     */
    bool is_recording() const { return recording_; }
    
private:
    void open_files(const std::string& symbol);
    void close_files();
    void write_headers();
    void flush_buffers();
    
    std::string generate_filename(const std::string& symbol, const std::string& type);
    
    HistoryRecorderConfig config_;
    bool recording_;
    std::string current_symbol_;
    
    // Output file streams
    std::ofstream trade_price_file_;
    std::ofstream mid_price_file_;
    std::ofstream orderbook_file_;
    
    // Timing
    std::chrono::steady_clock::time_point session_start_time_;
    std::chrono::steady_clock::time_point last_write_time_;
    int record_count_;
    
    // Track last written timestamps to avoid duplicates
    int64_t last_trade_timestamp_written_;
    int64_t last_mid_timestamp_written_;
};

} // namespace marketsim::monitor
