#include "history_recorder.h"
#include "monitor_config.h"
#include <filesystem>
#include <iostream>
#include <ctime>

namespace marketsim::monitor {

HistoryRecorder::HistoryRecorder(const HistoryRecorderConfig& config)
    : config_(config)
    , recording_(false)
    , record_count_(0)
    , last_trade_timestamp_written_(0)
    , last_mid_timestamp_written_(0)
    , last_ohlcv_timestamp_written_(0)
{
    // Create output directory if it doesn't exist
    std::filesystem::create_directories(config_.output_directory);
}

HistoryRecorder::~HistoryRecorder() {
    end_session();
}

void HistoryRecorder::start_session(const std::string& symbol) {
    if (recording_) {
        end_session();
    }
    
    current_symbol_ = symbol;
    recording_ = true;
    record_count_ = 0;
    session_start_time_ = std::chrono::steady_clock::now();
    last_write_time_ = session_start_time_;
    
    open_files(symbol);
    write_headers();
    flush_buffers();
    
    std::cout << "[HISTORY_RECORDER] Recording started for " << symbol << "\n";
}

void HistoryRecorder::record_status(const marketsim::exchange::StatusResponse& response) {
if (!recording_) {
    return;
}
    
auto now = std::chrono::steady_clock::now();
auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
    now - last_write_time_
).count();
    
// Only write at configured interval
if (elapsed < config_.write_interval_seconds) {
    return;  // Skip this cycle
}
    
last_write_time_ = now;
record_count_++;
    
// Calculate elapsed time since session start
auto session_elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
    now - session_start_time_
).count();
    
// Record trade price history (only new data points)
if (config_.record_trade_prices && trade_price_file_.is_open()) {
    int new_entries = 0;
        
    for (int i = 0; i < response.trade_price_history_size(); ++i) {
        const auto& tick = response.trade_price_history(i);
            
        // Skip if we already wrote this timestamp
        if (tick.timestamp_ms() <= last_trade_timestamp_written_) {
            continue;
        }
            
        new_entries++;
            
        // Convert timestamp to readable format
        auto tick_time = std::chrono::system_clock::time_point(
            std::chrono::milliseconds(tick.timestamp_ms())
        );
        auto time_t_tick = std::chrono::system_clock::to_time_t(tick_time);
        auto ms = tick.timestamp_ms() % 1000;
            
        std::tm tm_buf;
        #ifdef _WIN32
            localtime_s(&tm_buf, &time_t_tick);
        #else
            localtime_r(&time_t_tick, &tm_buf);
        #endif
            
        std::ostringstream timestamp_stream;
        timestamp_stream << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S")
                        << "." << std::setfill('0') << std::setw(3) << ms;
            
        trade_price_file_ << timestamp_stream.str() << ","
                         << tick.timestamp_ms() << ","
                         << tick.price() << "\n";
            
        // Update last written timestamp
        last_trade_timestamp_written_ = tick.timestamp_ms();
    }
        
    if (new_entries > 0) {
        trade_price_file_.flush();
    }
}
    
    
    
    
    // Record mid price history (only new data points)
    if (config_.record_mid_prices && mid_price_file_.is_open()) {
        int new_entries = 0;
        
        for (int i = 0; i < response.mid_price_history_size(); ++i) {
            const auto& tick = response.mid_price_history(i);
            
            // Skip if we already wrote this timestamp
            if (tick.timestamp_ms() <= last_mid_timestamp_written_) {
                continue;
            }
            
            new_entries++;
            
            // Convert timestamp to readable format
            auto tick_time = std::chrono::system_clock::time_point(
                std::chrono::milliseconds(tick.timestamp_ms())
            );
            auto time_t_tick = std::chrono::system_clock::to_time_t(tick_time);
            auto ms = tick.timestamp_ms() % 1000;
            
            std::tm tm_buf;
            #ifdef _WIN32
                localtime_s(&tm_buf, &time_t_tick);
            #else
                localtime_r(&time_t_tick, &tm_buf);
            #endif
            
            std::ostringstream timestamp_stream;
            timestamp_stream << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S")
                            << "." << std::setfill('0') << std::setw(3) << ms;
            
            mid_price_file_ << timestamp_stream.str() << ","
                           << tick.timestamp_ms() << ","
                           << tick.price() << ",";
            
            // Add current best bid/ask/spread for context
            if (response.has_current_orderbook()) {
                const auto& ob = response.current_orderbook();
                double best_bid = ob.bids_size() > 0 ? ob.bids(0).price() : 0.0;
                double best_ask = ob.asks_size() > 0 ? ob.asks(0).price() : 0.0;
                double spread = (best_bid > 0 && best_ask > 0) ? (best_ask - best_bid) : 0.0;
                
                mid_price_file_ << best_bid << ","
                              << best_ask << ","
                              << spread;
            } else {
                mid_price_file_ << "0.0,0.0,0.0";
            }
            mid_price_file_ << "\n";
            
            // Update last written timestamp
            last_mid_timestamp_written_ = tick.timestamp_ms();
        }
        
        if (new_entries > 0) {
            mid_price_file_.flush();
        }
    }
    
    // Record orderbook snapshot
    if (config_.record_orderbook_snapshots && orderbook_file_.is_open() 
        && response.has_current_orderbook()) {
        const auto& ob = response.current_orderbook();
        
        // Get current timestamp for orderbook
        auto sys_now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(sys_now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            sys_now.time_since_epoch()
        ).count() % 1000;
        
        std::tm tm_buf;
        #ifdef _WIN32
            localtime_s(&tm_buf, &time_t_now);
        #else
            localtime_r(&time_t_now, &tm_buf);
        #endif
        
        std::ostringstream timestamp_stream;
        timestamp_stream << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S")
                        << "." << std::setfill('0') << std::setw(3) << ms;
        
        // Write timestamp and basic info
        orderbook_file_ << timestamp_stream.str() << ","
                       << session_elapsed_ms << ",";
        
        // Write bid side (top 5)
        for (int i = 0; i < std::min(5, ob.bids_size()); ++i) {
            if (i > 0) orderbook_file_ << ";";
            orderbook_file_ << ob.bids(i).price() << ":"
                          << ob.bids(i).quantity() << ":"
                          << ob.bids(i).order_count();
        }
        orderbook_file_ << ",";
        
        // Write ask side (top 5)
        for (int i = 0; i < std::min(5, ob.asks_size()); ++i) {
            if (i > 0) orderbook_file_ << ";";
            orderbook_file_ << ob.asks(i).price() << ":"
                          << ob.asks(i).quantity() << ":"
                          << ob.asks(i).order_count();
        }
        orderbook_file_ << "\n";
        orderbook_file_.flush();
    }
}

void HistoryRecorder::record_ohlcv_bar(const marketsim::exchange::OHLCV& bar) {
    if (!recording_ || !config_.record_ohlcv) {
        return;
    }

    if (!ohlcv_file_.is_open()) {
        return;
    }

    // Skip if we already wrote this bar
    if (bar.timestamp() <= last_ohlcv_timestamp_written_) {
        return;
    }

    // Convert timestamp to readable format
    auto bar_time = std::chrono::system_clock::time_point(
        std::chrono::milliseconds(bar.timestamp())
    );
    auto time_t_bar = std::chrono::system_clock::to_time_t(bar_time);
    auto ms = bar.timestamp() % 1000;

    std::tm tm_buf;
    #ifdef _WIN32
        localtime_s(&tm_buf, &time_t_bar);
    #else
        localtime_r(&time_t_bar, &tm_buf);
    #endif

    std::ostringstream timestamp_stream;
    timestamp_stream << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S")
                    << "." << std::setfill('0') << std::setw(3) << ms;

    // Write OHLCV data
    ohlcv_file_ << timestamp_stream.str() << ","
                << bar.timestamp() << ","
                << bar.interval_seconds() << ","
                << bar.open() << ","
                << bar.high() << ","
                << bar.low() << ","
                << bar.close() << ","
                << bar.volume() << "\n";

    ohlcv_file_.flush();
    last_ohlcv_timestamp_written_ = bar.timestamp();
}

void HistoryRecorder::end_session() {
    if (!recording_) return;

    close_files();

    std::cout << "[HISTORY_RECORDER] Session ended. Recorded " << record_count_ << " snapshots.\n";

    recording_ = false;
}

void HistoryRecorder::open_files(const std::string& symbol) {
    if (config_.record_trade_prices) {
        std::string filename = generate_filename(symbol, "trade_prices");
        trade_price_file_.open(filename, std::ios::out | std::ios::trunc);
        if (!trade_price_file_.is_open()) {
            std::cerr << "[HISTORY_RECORDER] Failed to open: " << filename << "\n";
        }
    }

    if (config_.record_mid_prices) {
        std::string filename = generate_filename(symbol, "mid_prices");
        mid_price_file_.open(filename, std::ios::out | std::ios::trunc);
        if (!mid_price_file_.is_open()) {
            std::cerr << "[HISTORY_RECORDER] Failed to open: " << filename << "\n";
        }
    }

    if (config_.record_orderbook_snapshots) {
        std::string filename = generate_filename(symbol, "orderbook");
        orderbook_file_.open(filename, std::ios::out | std::ios::trunc);
        if (!orderbook_file_.is_open()) {
            std::cerr << "[HISTORY_RECORDER] Failed to open: " << filename << "\n";
        }
    }

    if (config_.record_ohlcv) {
        std::string filename = generate_filename(symbol, "ohlcv");
        ohlcv_file_.open(filename, std::ios::out | std::ios::trunc);
        if (!ohlcv_file_.is_open()) {
            std::cerr << "[HISTORY_RECORDER] Failed to open: " << filename << "\n";
        }
    }
}

void HistoryRecorder::close_files() {
    if (trade_price_file_.is_open()) trade_price_file_.close();
    if (mid_price_file_.is_open()) mid_price_file_.close();
    if (orderbook_file_.is_open()) orderbook_file_.close();
    if (ohlcv_file_.is_open()) ohlcv_file_.close();
}

void HistoryRecorder::write_headers() {
    if (trade_price_file_.is_open()) {
        trade_price_file_ << "timestamp,timestamp_ms,price\n";
    }

    if (mid_price_file_.is_open()) {
        mid_price_file_ << "timestamp,timestamp_ms,mid_price,best_bid,best_ask,spread\n";
    }

    if (orderbook_file_.is_open()) {
        orderbook_file_ << "timestamp,elapsed_ms,bids,asks\n";
        orderbook_file_ << "# Bids/Asks format: price:quantity:order_count;...\n";
    }

    if (ohlcv_file_.is_open()) {
        ohlcv_file_ << "timestamp,timestamp_ms,interval_seconds,open,high,low,close,volume\n";
    }
}

void HistoryRecorder::flush_buffers() {
    if (trade_price_file_.is_open()) trade_price_file_.flush();
    if (mid_price_file_.is_open()) mid_price_file_.flush();
    if (orderbook_file_.is_open()) orderbook_file_.flush();
    if (ohlcv_file_.is_open()) ohlcv_file_.flush();
}

std::string HistoryRecorder::generate_filename(const std::string& symbol, const std::string& type) {
    std::ostringstream filename;
    filename << config_.output_directory << "/" << symbol << "_" << type << ".csv";
    return filename.str();
}

} // namespace marketsim::monitor

