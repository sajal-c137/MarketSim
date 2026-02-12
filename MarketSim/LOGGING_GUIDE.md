# Logging Guide - Local Development

## ? Quick Start

Your application automatically logs everything to `logs/monitor.log` when you use:

```cpp
StatusMonitor::instance().set_output_mode(OutputMode::SILENT);
StatusMonitor::instance().enable_file_logging("logs/monitor.log");
StatusMonitor::instance().start_periodic_monitoring(std::chrono::seconds(2));
```

## ?? Watching Logs in Real-Time

### Terminal 1: Run Your Application
```bash
./out/build/x64-debug/MarketSim/test_io_handler.exe
```

### Terminal 2: Watch Logs
```bash
# Real-time log streaming (like tail -f)
tail -f logs/monitor.log
```

**Result:** Clean terminal showing only new log entries as they happen. No spam in app terminal!

---

## ?? Searching Logs with grep

All log files are plain text with format:
```
2026-02-12 14:23:15.123 [COMPONENT] Message details
```

### **Search by Thread Name**

```bash
# Show all activity from OrderIngestThread
grep "OrderIngestThread" logs/monitor.log

# Show only state changes for a thread
grep "OrderIngestThread.*state" logs/monitor.log

# Count activities per thread
grep -c "OrderIngestThread" logs/monitor.log
```

### **Search by Component/Socket**

```bash
# All exchange market data publisher events
grep "Exchange_MarketData" logs/monitor.log

# All subscriber events
grep "Subscriber" logs/monitor.log

# Find a specific socket
grep "Trader_Order_Client" logs/monitor.log
```

### **Search by Event Type**

```bash
# Find all state changes
grep "state.*changed" logs/monitor.log

# Find all connection events
grep "CONNECTED\|LISTENING\|DISCONNECTED" logs/monitor.log

# Find all errors
grep -i "error\|failed" logs/monitor.log

# Find socket send operations
grep "record_socket_send" logs/monitor.log

# Find socket receive operations  
grep "record_socket_receive" logs/monitor.log
```

### **Time-Based Searches**

```bash
# Show all logs from a specific hour
grep "^2026-02-12 14:" logs/monitor.log

# Show logs between 14:23 and 14:25
grep "^2026-02-12 14:2[345]" logs/monitor.log

# Show last 100 log lines (most recent)
tail -100 logs/monitor.log

# Show first 50 log lines (oldest)
head -50 logs/monitor.log
```

### **Count Statistics**

```bash
# Total number of log entries
wc -l logs/monitor.log

# Number of state changes
grep -c "state.*changed" logs/monitor.log

# Number of messages sent
grep -c "record_socket_send" logs/monitor.log

# Number of messages received
grep -c "record_socket_receive" logs/monitor.log

# Count by thread
grep -o "Thread.*:" logs/monitor.log | sort | uniq -c

# Count by socket
grep -o "Socket.*:" logs/monitor.log | sort | uniq -c
```

### **Extract Specific Information**

```bash
# Show only thread names and their states
grep "Thread" logs/monitor.log | grep -o "Thread[^:]*: [^ ]*" 

# Show only socket names and their states
grep "Socket" logs/monitor.log | grep -o "Socket[^:]*: [^ ]*"

# Get all unique components
grep -o "\[[A-Z_]*\]" logs/monitor.log | sort | uniq -c

# Count errors per component
grep -i "error" logs/monitor.log | grep -o "\[[A-Z_]*\]" | sort | uniq -c
```

---

## ?? Useful Grep Combinations

### **Monitor Thread Health**

```bash
# Find threads that went IDLE
grep "IDLE" logs/monitor.log

# Find threads that TERMINATED
grep "TERMINATED" logs/monitor.log

# Show all state transitions for a specific thread
grep "OrderIngestThread" logs/monitor.log | grep -E "RUNNING|IDLE|TERMINATED"

# Count how many tasks each thread completed
grep "increment_thread_tasks" logs/monitor.log | grep "OrderIngestThread" | wc -l
```

### **Monitor Socket Health**

```bash
# All socket connection attempts
grep "CONNECTED\|LISTENING" logs/monitor.log

# Find socket disconnections
grep "DISCONNECTED" logs/monitor.log

# Socket errors
grep "Socket.*error" logs/monitor.log

# Find which socket sent the most data
grep "record_socket_send" logs/monitor.log | grep -o "Socket[^:]*" | sort | uniq -c
```

### **Performance Monitoring**

```bash
# Count total messages sent
grep "record_socket_send" logs/monitor.log | wc -l

# Count total messages received
grep "record_socket_receive" logs/monitor.log | wc -l

# Messages per socket
grep "record_socket_send" logs/monitor.log | grep -o "Socket[^:]*" | sort | uniq -c

# Average message size (if tracked)
grep "record_socket_send.*bytes" logs/monitor.log
```

### **Debugging Issues**

```bash
# Find any errors or warnings
grep -iE "error|failed|warning|critical" logs/monitor.log

# Show context around errors (3 lines before/after)
grep -iE "error|failed" logs/monitor.log -B 3 -A 3

# Find when a specific order was processed
grep "ORD_12345" logs/monitor.log

# Timeline of a specific thread's execution
grep "OrderIngestThread" logs/monitor.log | head -20
```

---

## ?? Advanced: Piping to Create Summaries

### **Generate a Status Report**

```bash
# Create a quick report file
cat > report.txt << 'EOF'
=== Log Summary Report ===
EOF
echo "" >> report.txt
echo "Total Log Entries: $(wc -l < logs/monitor.log)" >> report.txt
echo "Threads Active: $(grep -c "RUNNING" logs/monitor.log)" >> report.txt
echo "Total Messages Sent: $(grep -c "record_socket_send" logs/monitor.log)" >> report.txt
echo "Total Messages Received: $(grep -c "record_socket_receive" logs/monitor.log)" >> report.txt
echo "Errors: $(grep -ic "error\|failed" logs/monitor.log)" >> report.txt
echo "" >> report.txt
echo "=== By Thread ===" >> report.txt
grep -o "Thread[^:]*" logs/monitor.log | sort | uniq -c >> report.txt
echo "" >> report.txt
echo "=== By Socket ===" >> report.txt
grep -o "Socket[^:]*" logs/monitor.log | sort | uniq -c >> report.txt

cat report.txt
```

### **Real-Time Stats (Monitor Continuously)**

```bash
# Watch log growth in real-time
watch -n 2 'echo "Entries: $(wc -l < logs/monitor.log) | Sent: $(grep -c send logs/monitor.log) | Recv: $(grep -c recv logs/monitor.log)"'
```

---

## ??? Grep Quick Reference

| Command | Purpose |
|---------|---------|
| `grep "pattern" file` | Find lines containing pattern |
| `grep -c "pattern" file` | Count matching lines |
| `grep -i "pattern" file` | Case-insensitive search |
| `grep -E "pat1\|pat2" file` | Search for multiple patterns (OR) |
| `grep -v "pattern" file` | Show lines NOT matching pattern |
| `grep "pattern" file \| wc -l` | Count matches |
| `grep -B 5 "pattern" file` | Show 5 lines before match |
| `grep -A 5 "pattern" file` | Show 5 lines after match |
| `grep -B 2 -A 2 "pattern" file` | Show 2 lines before and after |
| `tail -f file` | Watch file in real-time |
| `head -n 50 file` | Show first 50 lines |
| `tail -n 50 file` | Show last 50 lines |

---

## ?? Log Format

Each log line follows this format:

```
TIMESTAMP [COMPONENT] MESSAGE
```

Example:
```
2026-02-12 14:23:15.123 [Monitor #1] Threads: 2/3 active | Sockets: 2/2 connected | Msgs: ?1234 ?1200
2026-02-12 14:23:15.456 [OrderIngestThread] state changed: IDLE -> RUNNING
2026-02-12 14:23:15.789 [Exchange_Order_Server] record_socket_send: 256 bytes
2026-02-12 14:23:16.012 [Trader_Order_Client] record_socket_receive: 256 bytes
```

**Components you'll see:**
- `Monitor #N` - Status monitor reports
- `ThreadName` - Thread events
- `SocketName` - Socket events
- `[TIMESTAMP]` - System events

---

## ?? Pro Tips

### **Keep a Terminal Open for Logs**
```bash
# Split your terminal: top for app, bottom for logs
# Left: run app
./out/build/x64-debug/MarketSim/test_io_handler.exe

# Right: watch logs
tail -f logs/monitor.log
```

### **Log to Specific Directories**
```cpp
// Organize by date
StatusMonitor::instance().enable_file_logging("logs/2026-02-12.log");

// Or by component
StatusMonitor::instance().enable_file_logging("logs/exchange.log");
```

### **Parse Logs Programmatically**
```bash
# Export to CSV for analysis
grep "record_socket_send" logs/monitor.log | \
  awk -F'[ \[\]]' '{print $1,$2,$7}' > send_stats.csv

# Load in Excel or Python for analysis
python3 -c "import pandas as pd; df = pd.read_csv('send_stats.csv'); print(df.describe())"
```

### **Continuous Monitoring Loop**
```bash
# Monitor application state every 5 seconds
while true; do
  clear
  echo "=== MarketSim Status at $(date) ==="
  echo "Threads Running: $(grep -c RUNNING logs/monitor.log)"
  echo "Messages Sent: $(grep -c record_socket_send logs/monitor.log)"
  echo "Messages Recv: $(grep -c record_socket_receive logs/monitor.log)"
  echo "Errors: $(grep -ic error logs/monitor.log)"
  echo ""
  echo "Last 5 log entries:"
  tail -5 logs/monitor.log
  sleep 5
done
```

---

## ? Advantages of This Approach

? **No terminal spam** - App terminal stays clean  
? **All data preserved** - Every event is logged  
? **Searchable** - Use grep to find anything  
? **Historical** - Logs persist after app stops  
? **Simple** - No external tools or setup needed  
? **Free** - Standard Unix utilities (grep, tail, etc.)  
? **Scriptable** - Easy to automate log analysis  

---

## ?? Next Steps

When you're ready for advanced features:

1. **JSON Logging** - Add structured format for machine parsing
2. **Metrics Export** - Prometheus format for dashboards  
3. **Web Dashboard** - Flask UI for pretty visualization
4. **ELK Stack** - Enterprise-grade log aggregation

But for local development, grep is all you need! ??
