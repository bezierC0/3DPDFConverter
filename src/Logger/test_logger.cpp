/**
 * @file test_logger.cpp
 * @brief Simple verification test for Logger module
 * 
 * This file verifies that the Logger module compiles correctly
 * and basic functionality works as intended.
 */

#include "Logger.h"
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>
#include <filesystem>
#include <fstream>

using namespace Logger;

void testLogLevelEnum() {
    // Test LogLevel enumeration
    LogLevel debug = LogLevel::DEBUG;
    LogLevel info = LogLevel::INFO;
    LogLevel warn = LogLevel::WARN;
    LogLevel error = LogLevel::ERROR;
    
    assert(static_cast<int>(debug) == 0);
    assert(static_cast<int>(info) == 1);
    assert(static_cast<int>(warn) == 2);
    assert(static_cast<int>(error) == 3);
    
    std::cout << "✓ LogLevel enumeration test passed" << std::endl;
}

void testLoggerSingleton() {
    // Test singleton pattern
    Logger& logger1 = Logger::GetInstance();
    Logger& logger2 = Logger::GetInstance();
    
    // Both references should point to the same instance
    assert(&logger1 == &logger2);
    
    std::cout << "✓ Logger singleton test passed" << std::endl;
}

void testBasicLogging() {
    // Test basic logging functionality
    Logger& logger = Logger::GetInstance();
    
    // Initialize with test log file
    std::string testLogFile = "test_logger.log";
    logger.Initialize(testLogFile, false);
    
    // Log messages at different levels
    logger.Debug("TestModule", "This is a debug message");
    logger.Info("TestModule", "This is an info message");
    logger.Warn("TestModule", "This is a warning message");
    logger.Error("TestModule", "This is an error message");
    
    // Flush to ensure all messages are written
    logger.Flush();
    
    // Verify log file exists
    assert(std::filesystem::exists(testLogFile));
    
    // Read log file and verify content
    std::ifstream logFile(testLogFile);
    assert(logFile.is_open());
    
    std::string line;
    int lineCount = 0;
    bool foundDebug = false;
    bool foundInfo = false;
    bool foundWarn = false;
    bool foundError = false;
    
    while (std::getline(logFile, line)) {
        lineCount++;
        if (line.find("[DEBUG]") != std::string::npos && line.find("debug message") != std::string::npos) {
            foundDebug = true;
        }
        if (line.find("[INFO]") != std::string::npos && line.find("info message") != std::string::npos) {
            foundInfo = true;
        }
        if (line.find("[WARN]") != std::string::npos && line.find("warning message") != std::string::npos) {
            foundWarn = true;
        }
        if (line.find("[ERROR]") != std::string::npos && line.find("error message") != std::string::npos) {
            foundError = true;
        }
    }
    
    logFile.close();
    
    // Verify all messages were logged
    assert(foundDebug);
    assert(foundInfo);
    assert(foundWarn);
    assert(foundError);
    
    // Clean up
    logger.Close();
    std::filesystem::remove(testLogFile);
    
    std::cout << "✓ Basic logging test passed" << std::endl;
}

void testLogLevelFiltering() {
    // Test log level filtering
    Logger& logger = Logger::GetInstance();
    
    std::string testLogFile = "test_logger_filter.log";
    logger.Initialize(testLogFile, false);
    
    // Set minimum log level to WARN
    logger.SetLogLevel(LogLevel::WARN);
    
    // Log messages at different levels
    logger.Debug("TestModule", "This debug should be filtered");
    logger.Info("TestModule", "This info should be filtered");
    logger.Warn("TestModule", "This warning should appear");
    logger.Error("TestModule", "This error should appear");
    
    logger.Flush();
    
    // Read log file and verify only WARN and ERROR messages are present
    std::ifstream logFile(testLogFile);
    assert(logFile.is_open());
    
    std::string line;
    bool foundDebug = false;
    bool foundInfo = false;
    bool foundWarn = false;
    bool foundError = false;
    
    while (std::getline(logFile, line)) {
        if (line.find("debug should be filtered") != std::string::npos) {
            foundDebug = true;
        }
        if (line.find("info should be filtered") != std::string::npos) {
            foundInfo = true;
        }
        if (line.find("warning should appear") != std::string::npos) {
            foundWarn = true;
        }
        if (line.find("error should appear") != std::string::npos) {
            foundError = true;
        }
    }
    
    logFile.close();
    
    // Verify filtering worked correctly
    assert(!foundDebug);  // Should be filtered
    assert(!foundInfo);   // Should be filtered
    assert(foundWarn);    // Should appear
    assert(foundError);   // Should appear
    
    // Clean up
    logger.Close();
    std::filesystem::remove(testLogFile);
    
    std::cout << "✓ Log level filtering test passed" << std::endl;
}

void testThreadSafeLogging() {
    // Test thread-safe logging
    Logger& logger = Logger::GetInstance();
    
    std::string testLogFile = "test_logger_threadsafe.log";
    logger.Initialize(testLogFile, false);
    logger.SetLogLevel(LogLevel::INFO);
    
    // Create multiple threads that log simultaneously
    const int numThreads = 5;
    const int messagesPerThread = 10;
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&logger, i, messagesPerThread]() {
            for (int j = 0; j < messagesPerThread; ++j) {
                logger.Info("Thread" + std::to_string(i), 
                           "Message " + std::to_string(j));
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    logger.Flush();
    
    // Count lines in log file
    std::ifstream logFile(testLogFile);
    assert(logFile.is_open());
    
    int lineCount = 0;
    std::string line;
    while (std::getline(logFile, line)) {
        if (line.find("[INFO]") != std::string::npos) {
            lineCount++;
        }
    }
    
    logFile.close();
    
    // Verify all messages were logged (excluding initialization message)
    // We expect numThreads * messagesPerThread messages
    assert(lineCount >= numThreads * messagesPerThread);
    
    // Clean up
    logger.Close();
    std::filesystem::remove(testLogFile);
    
    std::cout << "✓ Thread-safe logging test passed" << std::endl;
}

void testLogMessageFormat() {
    // Test log message formatting
    Logger& logger = Logger::GetInstance();
    
    std::string testLogFile = "test_logger_format.log";
    logger.Initialize(testLogFile, false);
    logger.SetLogLevel(LogLevel::INFO);
    
    logger.Info("TestModule", "Test message");
    logger.Flush();
    
    // Read log file and verify format
    std::ifstream logFile(testLogFile);
    assert(logFile.is_open());
    
    std::string line;
    bool foundCorrectFormat = false;
    
    while (std::getline(logFile, line)) {
        if (line.find("Test message") != std::string::npos) {
            // Verify format: [YYYY-MM-DD HH:MM:SS] [LEVEL] [Module] Message
            // Check for timestamp pattern
            bool hasTimestamp = (line.find("[20") != std::string::npos || 
                                line.find("[19") != std::string::npos);
            bool hasLevel = (line.find("[INFO]") != std::string::npos);
            bool hasModule = (line.find("[TestModule]") != std::string::npos);
            bool hasMessage = (line.find("Test message") != std::string::npos);
            
            if (hasTimestamp && hasLevel && hasModule && hasMessage) {
                foundCorrectFormat = true;
            }
        }
    }
    
    logFile.close();
    
    assert(foundCorrectFormat);
    
    // Clean up
    logger.Close();
    std::filesystem::remove(testLogFile);
    
    std::cout << "✓ Log message format test passed" << std::endl;
}

void testFileRotation() {
    // Test file rotation (simplified test - we won't create 10MB files)
    Logger& logger = Logger::GetInstance();
    
    std::string testLogFile = "test_logger_rotation.log";
    logger.Initialize(testLogFile, false);
    logger.SetLogLevel(LogLevel::INFO);
    
    // Log some messages
    for (int i = 0; i < 100; ++i) {
        logger.Info("TestModule", "Message " + std::to_string(i));
    }
    
    logger.Flush();
    
    // Verify log file exists
    assert(std::filesystem::exists(testLogFile));
    
    // Clean up
    logger.Close();
    std::filesystem::remove(testLogFile);
    
    // Note: Full rotation test would require creating 10MB+ files
    // which is impractical for a quick verification test
    
    std::cout << "✓ File rotation test passed (basic verification)" << std::endl;
}

int main() {
    std::cout << "Running Logger verification tests..." << std::endl;
    std::cout << std::endl;
    
    try {
        testLogLevelEnum();
        testLoggerSingleton();
        testBasicLogging();
        testLogLevelFiltering();
        testThreadSafeLogging();
        testLogMessageFormat();
        testFileRotation();
        
        std::cout << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "All Logger tests passed successfully!" << std::endl;
        std::cout << "========================================" << std::endl;
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
