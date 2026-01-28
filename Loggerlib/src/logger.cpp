#include "logger.h"
#include <iostream>
#include <stdexcept>
#include <filesystem>

Logger::Logger(): initialized(false) {}

bool Logger::initializate(const std::string& filename, LogLvl defaultLogLvl) {
    if (initialized == true) {
        return false;
    }
    
    filename_ = filename;
    defaultLogLvl_ = defaultLogLvl;
    try {
        std::filesystem::path filepath(filename_);
        auto parent_path = filepath.parent_path();
        
        if (!parent_path.empty() && !std::filesystem::exists(parent_path)) {
            std::filesystem::create_directories(parent_path);
        }
        
        logFile_.open(filename_, std::ios::app);
        if (!logFile_.is_open()) {
            throw std::runtime_error("Cannot open file for writing");
        }
        
        initialized = true;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize logger: " << e.what() << std::endl;
        return false;
    }

    return true;
}

void Logger::setLogLvl(LogLvl lvl) {
    defaultLogLvl_ = lvl;
}

bool Logger::log(const std::string& msg, LogLvl lvl) {
    if (initialized == false) {
        std::cerr << "Logger not initializated or not running" << std::endl;
    }

    if (static_cast<int>(lvl) < static_cast<int>(defaultLogLvl_)) {
        return false;
    }
    LogMessage lmsg(msg,lvl);
    writeMessage(lmsg);
    
    return true;
}

void Logger::writeMessage(const LogMessage& msg) {
    if (logFile_.is_open() == false) {
        throw std::runtime_error("Log file is not open.");
    }

    std::string timeFormat = TimetoSting(msg.time);
    std::string lvlStr = LogMessageToString(msg.level);

    logFile_ << "[" << timeFormat << "] " << "[" << lvlStr << "] " << msg.message << std::endl;
    logFile_.flush();
    if (logFile_.fail()) {
        throw std::runtime_error("Failed to write log to file");
    }
}

std::string Logger::TimetoSting(const std::chrono::system_clock::time_point& p) const {
    auto time = std::chrono::system_clock::to_time_t(p);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(p.time_since_epoch()) % 1000;
    std::stringstream ss;
    
    std::tm tmbuf;
    localtime_r(&time, &tmbuf);

    ss << std::put_time(&tmbuf, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << ms.count();

    return ss.str();
}

bool Logger::isInitializate() {
    return initialized;
}

Logger::~Logger() {
    abort();
}

void Logger::abort() {
    if (initialized == false) {
        return;
    }
    try {
        if (logFile_.is_open() == true) {
            logFile_.close();
        }
    } catch(const std::exception& e) {
        std::cerr << "Error during logger abort: " << e.what() << std::endl;
    }
}