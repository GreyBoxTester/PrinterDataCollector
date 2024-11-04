#include "Service.h"


Service::Service(std::chrono::steady_clock::duration updateDelay)
    : updateDelay_(updateDelay)
{}

Service::~Service()
{
    stop();
}

void Service::run()
{
    std::unique_lock lock(mutex_);
    running_ = true;

    onStartup();
    while (running_)
    {
        if (reload_) { onReload(); reload_ = false; }
        onUpdate();
        cv_.wait_for(lock, updateDelay_, [this]() { return !running_ || reload_; });
    }
    onShutdown();
}

void Service::stop()
{
    std::unique_lock lock(mutex_);
    running_ = false;
    cv_.notify_all();
}

void Service::reload()
{
    std::unique_lock lock(mutex_);
    reload_ = true;
    cv_.notify_all();
}
