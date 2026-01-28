#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <queue>
#include <thread>
#include <condition_variable>
#include <atomic>

enum class LogLvl{
    DEBUG = 0,
    INFO = 1,
    ERROR = 2,
};

inline std::string LogMessageToString(LogLvl lvl) {
    switch(lvl) {
        case LogLvl::DEBUG: return "DEBUG";
        case LogLvl::INFO: return "INFO";
        case LogLvl::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

struct LogMessage {
    std::string message;
    LogLvl level;
    std::chrono::system_clock::time_point time;
    LogMessage(): message(""), level(LogLvl::INFO), time(std::chrono::system_clock::now()) {}
    LogMessage(const std::string& msg, LogLvl lvl): message(msg), level(lvl), time(std::chrono::system_clock::now()) {}
};

class Logger {
    private:
        std::string filename_;
        LogLvl defaultLogLvl_;
        std::ofstream logFile_;

        bool initialized;
        public:

        Logger();
        
        bool initializate(const std::string& filename, LogLvl defaultLogLvl);
    
        void setLogLvl(LogLvl lvl);

        bool log(const std::string& message, LogLvl lvl);

        bool isInitializate();

        void abort();
        
        ~Logger();
    private:
        Logger(const Logger&) = delete;
        Logger operator=(const Logger&) = delete;
        
        void writeMessage(const LogMessage& msg);

        std::string TimetoSting(const std::chrono::system_clock::time_point& p) const;
        
};