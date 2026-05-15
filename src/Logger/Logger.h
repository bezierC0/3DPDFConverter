#ifndef LOGGER_H
#define LOGGER_H

#include "Logger_global.h"
#include <string>
#include <fstream>
#include <mutex>
#include <memory>
#include <cstdint>

namespace Logger {

/**
 * @brief Log level enumeration
 * 
 * Validates: Requirements 15.1
 */
enum class LogLevel {
    Debug = 0,
    Info = 1,
    Warn = 2,
    Error = 3
};

/**
 * @brief Thread-safe singleton logger with file rotation and console output
 * 
 * The Logger class provides centralized logging functionality with:
 * - Singleton pattern for global access
 * - Thread-safe logging with mutex protection
 * - File logging with automatic rotation (10MB limit, keep 5 files)
 * - Console logging for CLI mode
 * - Formatted log messages with timestamp, level, module name, and message
 * 
 * Validates: Requirements 15.1, 15.2, 15.3, 15.4, 15.5, 15.6, 15.7
 */
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

class LOGGER_EXPORT Logger {
public:
    /**
     * @brief Get the singleton Logger instance
     * @return Reference to the Logger instance
     */
    static Logger& GetInstance();

    // Delete copy constructor and assignment operator
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief Initialize the logger with file path and console mode
     * @param logFilePath Path to the log file (default: "3DPDFConverter.log")
     * @param enableConsole Enable console output (default: false)
     */
    void Initialize(const std::string& logFilePath = "3DPDFConverter.log", bool enableConsole = false);

    /**
     * @brief Set the minimum log level to output
     * @param level Minimum log level (messages below this level are filtered)
     */
    void SetLogLevel(LogLevel level);

    /**
     * @brief Log a message with specified level, module, and content
     * @param level Log level (DEBUG, INFO, WARN, ERROR)
     * @param moduleName Name of the module generating the log
     * @param message Log message content
     * 
     * Validates: Requirements 15.2
     */
    void Log(LogLevel level, const std::string& moduleName, const std::string& message);

    /**
     * @brief Convenience method for DEBUG level logging
     */
    void Debug(const std::string& moduleName, const std::string& message);

    /**
     * @brief Convenience method for INFO level logging
     */
    void Info(const std::string& moduleName, const std::string& message);

    /**
     * @brief Convenience method for WARN level logging
     */
    void Warn(const std::string& moduleName, const std::string& message);

    /**
     * @brief Convenience method for ERROR level logging
     */
    void Error(const std::string& moduleName, const std::string& message);

    /**
     * @brief Flush the log file to disk
     */
    void Flush();

    /**
     * @brief Close the log file
     */
    void Close();

private:
    Logger();
    ~Logger();

    /**
     * @brief Format log message with timestamp, level, module name, and message
     * @param level Log level
     * @param moduleName Module name
     * @param message Message content
     * @return Formatted log message string
     * 
     * Validates: Requirements 15.2
     */
    std::string FormatLogMessage(LogLevel level, const std::string& moduleName, const std::string& message);

    /**
     * @brief Convert LogLevel enum to string representation
     * @param level Log level
     * @return String representation (DEBUG, INFO, WARN, ERROR)
     */
    std::string LogLevelToString(LogLevel level);

    /**
     * @brief Get current timestamp in format YYYY-MM-DD HH:MM:SS
     * @return Formatted timestamp string
     */
    std::string GetTimestamp();

    /**
     * @brief Check if log file rotation is needed and perform rotation
     * 
     * Rotates log files when current file exceeds 10MB.
     * Keeps maximum 5 log files (3DPDFConverter.log, 3DPDFConverter.1.log, ..., 3DPDFConverter.4.log)
     * 
     * Validates: Requirements 15.6, 15.7
     */
    void RotateLogFileIfNeeded();

    /**
     * @brief Get the size of the current log file
     * @return File size in bytes
     */
    size_t GetLogFileSize();

    /**
     * @brief Perform log file rotation
     * 
     * Renames existing log files:
     * 3DPDFConverter.log -> 3DPDFConverter.1.log
     * 3DPDFConverter.1.log -> 3DPDFConverter.2.log
     * ...
     * 3DPDFConverter.4.log is deleted
     */
    void PerformRotation();

    std::mutex m_mutex;                 ///< Mutex for thread-safe logging
    std::ofstream m_logFile;            ///< Output file stream for log file
    std::string m_logFilePath;          ///< Path to the current log file
    bool m_consoleEnabled;              ///< Flag for console output
    LogLevel m_minLogLevel;             ///< Minimum log level to output
    bool m_initialized;                 ///< Initialization flag

    static constexpr size_t MAX_LOG_FILE_SIZE = 10 * 1024 * 1024;  ///< 10MB
    static constexpr int MAX_LOG_FILES = 5;                         ///< Keep 5 log files
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

} // namespace Logger

#endif // LOGGER_H
