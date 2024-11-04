#pragma once
#include <mutex>
#include <condition_variable>
#include <string>
#include <chrono>

class Service
{
    friend class ServiceRunner;
public:
    Service(std::chrono::steady_clock::duration updateDelay);
    ~Service();
private:
    void run();
    void stop();
    void reload();
protected:    
    virtual void onStartup() = 0;
    virtual void onUpdate() = 0;
    virtual void onShutdown() = 0;
    virtual void onReload() = 0;
private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::chrono::steady_clock::duration updateDelay_; 
    bool running_ = false;
    bool reload_ = false;
};