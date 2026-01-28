#include "../Loggerlib/include/logger.h"
#include <iostream>
#include <limits>
#include <sstream>
#include <cstdlib>
#include <thread>
#include <memory>
#include <string>
#include <csignal>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>

class LogQueue {
    std::queue<LogMessage> queue_;
    std::mutex Qmutex;
    std::condition_variable QueueCV_;
    std::atomic<bool> fstop{false};
public:
    
void push(const std::string& message, LogLvl lvl) {
        {
            std::lock_guard<std::mutex> lock(Qmutex);
            queue_.emplace(message,lvl);
        }
        QueueCV_.notify_one();
    }
    
    bool pop(LogMessage& msg){
        std::unique_lock<std::mutex> lock(Qmutex);
        QueueCV_.wait(lock, [this]{
            return (queue_.empty() == false || fstop == true);
        });
        if (fstop == true || queue_.empty()) {
            return false;
        }
        
        msg = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    void stop() {
        fstop = true;
        QueueCV_.notify_all();
    }

    bool empty() {
        std::scoped_lock lock(Qmutex);
        return queue_.empty();
    }
};

class Logapp {
private:
    LogLvl defaultLevel;
    std::string filename;

    Logger logger_;    
    LogQueue messageQueue;
    
    std::vector<std::thread> workingThreads;
    std::atomic<bool> running{false};
    std::atomic<bool> stopRequested{false};


public:

    Logapp(): defaultLevel(LogLvl::INFO){}
    
    ~Logapp() {
        abort();
    }

    bool initializate() {
        std::cout<< "=== Logger Initialization ==="<<std::endl;
        
        while (true) {

            std::cout << "Enter log file name(or press Enter for 'app.txt'):";
            std::getline(std::cin, filename);
            if (filename.empty() == true) {
                filename = "app.txt";
                std::cout << "File name:" << filename << std::endl;
                break;
            } else {
                break;
            }
        }

        while(true) {
            std::cout << "\nSelect default log level" << std::endl;
            std::cout << " 1.DEBUG (all message) " << std::endl;
            std::cout << " 2.INFO (info and errors)" << std::endl;
            std::cout << " 3.ERROR (errors only)" << std::endl;
            std::cout << "Enter choice (default 2.INFO):"<<std::endl;
            std::string lvlChoice;
            std::getline(std::cin, lvlChoice);
            if (lvlChoice.empty() == true) {
                std::cout << "Using default level: INFO" << std::endl;
                defaultLevel = LogLvl::INFO;
                break;
            }
            if (lvlChoice == "1" || lvlChoice == "DEBUG") {
                std::cout << "Using level: DEBUG" << std::endl;
                defaultLevel = LogLvl::DEBUG;
                break;
            }else if (lvlChoice == "2" || lvlChoice == "INFO") {
                std::cout << "Using level: INFO" << std::endl;
                defaultLevel = LogLvl::INFO;
                break;
            } else if (lvlChoice == "3" || lvlChoice == "ERRIR") {
                std::cout << "Using level: ERRORS" << std::endl;
                defaultLevel = LogLvl::ERROR;
                break;
            } else {
                std::cout << "Invalid choice. Please try again." << std::endl;
            }
        }

        std::cout << "Initialization logger" << std::endl;    
   
        if (logger_.initializate(filename, defaultLevel) == false) {
            std::cerr << "Failed to initialize logger!" << std::endl;
            return false;
        }

        startWorkingThreads();

        std::cout << "\n=== Logger Started ===" << std::endl;
        std::cout << "Log File: " << filename << std::endl;
        std::cout << "Default message lvl: " << LogMessageToString(defaultLevel)<< std::endl;
        std::cout << "\nCommands: " << std::endl;
        std::cout << "status             - Show current settings" << std::endl;
        std::cout << "setlevel           - Change default log level" << std::endl;
        std::cout << "message LVL text   - Log message with optional level" << std::endl;
        std::cout << "help               - Show help menu" << std::endl;
        std::cout << "quit               - Quit" << std::endl;
        std::cout << "\nEnter command:" << std::endl;
        
        return true;
    }

    void startWorkingThreads() {
        size_t numThreads = std::thread::hardware_concurrency();
        if (numThreads == 0) numThreads = 2;
        for (size_t i = 0; i < numThreads; i++) {
            workingThreads.emplace_back(&Logapp::workingThreadsFunc, this, i);
        }
        running = true;
    }

    void workingThreadsFunc(int threadId) {
        while (stopRequested == false) {
            LogMessage lgmsg;
            if (messageQueue.pop(lgmsg) == true) {
                if (logger_.log(lgmsg.message, lgmsg.level) == true) {
                    std:: cout << "Message Logged" << std::endl;
                }
            } else {
                std::cout << "Can't log message. Low recording level." << std::endl;
                break;
            }
        } 
    }

    void printHelp() {
        std::cout << "\n=== Logger Help ===" << std::endl;
        std::cout << "\nCommands:" << std::endl;
        std::cout << "status             - Show current settings" << std::endl;
        std::cout << "setlevel           - Change default log level" << std::endl;
        std::cout << "message LVL text   - Log message with optional level" << std::endl;
        std::cout << "help               - Show help menu" << std::endl;
        std::cout << "quit               - Quit" << std::endl;
        std::cout << "\nEnter command:" << std::endl;
    }
    void printStatus() {
        std::cout << "\n=== Logger Status ===" << std::endl;
        
        if (logger_.isInitializate() == true) {
            std::cout << "✓ Logger is initializate" << std::endl;
        } else {
            std::cout << "❌ Logger is not inintializate" << std::endl;
        }
        std::cout << "\nfilename: " << filename << std::endl;
        std::cout << "Default message lvl: " << LogMessageToString(defaultLevel) << std::endl;
    }

    void SetLevel(const std::string input) {
        std::stringstream ss(input.substr(9));
        std::string levelStr;
        ss >> levelStr;
        
        LogLvl newlvl;
        if (levelStr == "DEBUG") {
            newlvl = LogLvl::DEBUG;
        } else if (levelStr == "INFO") {
            newlvl = LogLvl::INFO;
        } else if (levelStr == "ERROR") {
            newlvl = LogLvl::ERROR;
        } else {
            std::cout << " Invalid level. Use DEBUG, INFO, ERROR" << std::endl;
            return;
        }

        logger_.setLogLvl(newlvl);
        defaultLevel = newlvl;
        std::cout << "Log lvl set to " << levelStr << std::endl;
    }

    void writeLogMessage(const std::string input) {
        std::stringstream ss(input.substr(8));
        std::string levelStr, message;
        ss>>levelStr;
        LogLvl msglvl = defaultLevel;

        if (levelStr == "DEBUG" || levelStr == "INFO" || levelStr == "ERROR") {
            if (levelStr == "DEBUG") {
                msglvl = LogLvl::DEBUG;
            } else if (levelStr == "INFO") {
                msglvl = LogLvl::INFO;
            } else if (levelStr == "ERROR") {
                msglvl = LogLvl::ERROR;
            }
            std::getline(ss, message);
            if (message.empty() == false || message[0] == ' ') {
                message = message.substr(1);
            }
        } else {
            message = levelStr;
            std::string rest;
            while (ss >> rest) {
                message += " " + rest;
            }
            msglvl = defaultLevel;
        }

        if (message.empty() == true) {
            std::cout << "Message empty" << std::endl;
            return;
        }
        messageQueue.push(message, msglvl);

    }

    void stop() {
        if (running == false) {
            return;
        }
        stopRequested = true;
        messageQueue.stop();
        for (auto& thread : workingThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        running = false;
        std::cout << "All process stoped." << std::endl;
    }

    void run() {
        std::string input;
        
        while(true) {
            std::getline(std::cin, input);

            if (std::cin.fail() || std::cin.eof()) {
                std::cout << "\nExiting..." << std::endl;
                break;
            }

            if (input.empty()){
                continue;
            }
            if (input == "quit" || input == "exit" || input == "q") {
                running = false;
                break;
            } else if (input == "help") {
                printHelp();
            } else if (input == "status") {
                printStatus();
            } else if (input.find("setlevel ") == 0) {
                SetLevel(input);
            } else if (input.find("message ") == 0) {
                writeLogMessage(input);
            } else {
            }
        }
    }
};

int main () {
    std::cout << "\033[1;36m" << "=== Multi-threaded Logger Application ===" << "\033[0m" << std::endl;
    std::cout << "Press Ctrl+C to exit at any time\n" << std::endl;
    try {
        Logapp app;
        
        if (app.initializate() == false) {
            return 1;
        }
        app.run();
        app.stop();
        

    } catch (const std::exception& e) {
        std::cerr << "\n ❌ Error: " << e.what() << std::endl;
        return 1; 
    }
    return 0;
}