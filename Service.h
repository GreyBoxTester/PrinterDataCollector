#include <mutex>
#include <atomic>
#include <condition_variable>
#include <string>
#include <chrono>

class Service
{
public:
    Service(std::chrono::steady_clock::duration updateDelay);
    ~Service();
    void run();
private:
    void stop();
    void restart();
    bool daemonize();
    static void signalHandler(int sig);
protected:    
    virtual void onStartup() = 0;
    virtual void onUpdate() = 0;
    virtual void onShutdown() = 0;
    virtual void onRestart() = 0;
private:
    std::mutex mutex_;
    std::condition_variable cv_;
    bool running_ = false;
    bool restart_ = false;
    std::chrono::steady_clock::duration updateDelay_; 
private:
    static Service* serviceInstance_;
};