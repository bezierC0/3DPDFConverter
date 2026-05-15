#include "Logger.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

namespace Logger {

Logger::Logger()
    : m_consoleEnabled(false)
    , m_minLogLevel(LogLevel::Info)
    , m_initialized(false)
{
}

Logger::~Logger() {
    Close();
}

Logger& Logger::GetInstance() {
    static Logger instance;
    return instance;
}

void Logger::Initialize(const std::string& logFilePath, bool enableConsole) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Close existing log file if open
    if (m_logFile.is_open()) {
        m_logFile.close();
    }
    
    m_logFilePath = logFilePath;
    m_consoleEnabled = enableConsole;
    
    // Open log file in append mode
    m_logFile.open(m_logFilePath, std::ios::out | std::ios::app);
    
    if (!m_logFile.is_open()) {
        std::cerr << "Failed to open log file: " << m_logFilePath << std::endl;
        return;
    }
    
    m_initialized = true;
    
    // Log initialization message
    Log(LogLevel::Info, "Logger", "Logger initialized: " + m_logFilePath);
}

void Logger::SetLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_minLogLevel = level;
}

void Logger::Log(LogLevel level, const std::string& moduleName, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Filter messages below minimum log level
    if (level < m_minLogLevel) {
        return;
    }
    
    // Check if rotation is needed before writing
    if (m_initialized && m_logFile.is_open()) {
        RotateLogFileIfNeeded();
    }
    
    // Format the log message
    std::string formattedMessage = FormatLogMessage(level, moduleName, message);
    
    // Write to file if initialized
    if (m_initialized && m_logFile.is_open()) {
        m_logFile << formattedMessage << std::endl;
        m_logFile.flush();  // Ensure immediate write
    }
    
    // Write to console if enabled
    if (m_consoleEnabled) {
        if (level == LogLevel::Error) {
            std::cerr << formattedMessage << std::endl;
        } else {
            std::cout << formattedMessage << std::endl;
        }
    }
}

void Logger::Debug(const std::string& moduleName, const std::string& message) {
    Log(LogLevel::Debug, moduleName, message);
}

void Logger::Info(const std::string& moduleName, const std::string& message) {
    Log(LogLevel::Info, moduleName, message);
}

void Logger::Warn(const std::string& moduleName, const std::string& message) {
    Log(LogLevel::Warn, moduleName, message);
}

void Logger::Error(const std::string& moduleName, const std::string& message) {
    Log(LogLevel::Error, moduleName, message);
}

void Logger::Flush() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_logFile.is_open()) {
        m_logFile.flush();
    }
}

void Logger::Close() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_logFile.is_open()) {
        Log(LogLevel::Info, "Logger", "Logger closing");
        m_logFile.close();
    }
    m_initialized = false;
}

std::string Logger::FormatLogMessage(LogLevel level, const std::string& moduleName, const std::string& message) {
    std::ostringstream oss;
    oss << "[" << GetTimestamp() << "] "
        << "[" << LogLevelToString(level) << "] "
        << "[" << moduleName << "] "
        << message;
    return oss.str();
}

std::string Logger::LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info:  return "INFO";
        case LogLevel::Warn:  return "WARN";
        case LogLevel::Error: return "ERROR";
        default:              return "UNKNOWN";
    }
}

std::string Logger::GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    
    std::tm tm_now;
#ifdef _WIN32
    localtime_s(&tm_now, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_now);
#endif
    
    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void Logger::RotateLogFileIfNeeded() {
    // Check if file size exceeds limit
    size_t fileSize = GetLogFileSize();
    
    if (fileSize >= MAX_LOG_FILE_SIZE) {
        // Close current file
        m_logFile.close();
        
        // Perform rotation
        PerformRotation();
        
        // Reopen log file
        m_logFile.open(m_logFilePath, std::ios::out | std::ios::app);
        
        if (!m_logFile.is_open()) {
            std::cerr << "Failed to reopen log file after rotation: " << m_logFilePath << std::endl;
        }
    }
}

size_t Logger::GetLogFileSize() {
    if (!m_logFile.is_open()) {
        return 0;
    }
    
    // Get current position
    std::streampos currentPos = m_logFile.tellp();
    
    // Seek to end
    m_logFile.seekp(0, std::ios::end);
    std::streampos endPos = m_logFile.tellp();
    
    // Restore position
    m_logFile.seekp(currentPos);
    
    return static_cast<size_t>(endPos);
}

void Logger::PerformRotation() {
    namespace fs = std::filesystem;
    
    try {
        // Delete the oldest log file (3DPDFConverter.4.log)
        std::string oldestFile = m_logFilePath + "." + std::to_string(MAX_LOG_FILES - 1);
        if (fs::exists(oldestFile)) {
            fs::remove(oldestFile);
        }
        
        // Rotate existing log files
        // 3DPDFConverter.3.log -> 3DPDFConverter.4.log
        // 3DPDFConverter.2.log -> 3DPDFConverter.3.log
        // 3DPDFConverter.1.log -> 3DPDFConverter.2.log
        for (int i = MAX_LOG_FILES - 2; i >= 1; --i) {
            std::string oldName = m_logFilePath + "." + std::to_string(i);
            std::string newName = m_logFilePath + "." + std::to_string(i + 1);
            
            if (fs::exists(oldName)) {
                fs::rename(oldName, newName);
            }
        }
        
        // Rename current log file to 3DPDFConverter.1.log
        std::string firstBackup = m_logFilePath + ".1";
        if (fs::exists(m_logFilePath)) {
            fs::rename(m_logFilePath, firstBackup);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error during log rotation: " << e.what() << std::endl;
    }
}

} // namespace Logger
