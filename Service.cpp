#include "Service.h"
#include <cstring>
#include <csignal>
#include <sys/stat.h>
#include <syslog.h>
#include <iostream>


using namespace std::string_literals;

Service* Service::serviceInstance_ = nullptr;

Service::Service(std::chrono::steady_clock::duration updateDelay)
    : updateDelay_(updateDelay)
{
    if (serviceInstance_) { throw std::logic_error("only one Service object is allowed"); }
    serviceInstance_ = this;
}

Service::~Service()
{
    stop();
    serviceInstance_ = nullptr;
}

void Service::run()
{
    if (running_) { return; }

    if (!daemonize()) { return; }
    
    onStartup();
    std::unique_lock lock(mutex_);
    while (running_)
    {
        if (restart_) { onRestart(); restart_ = false; }
        onUpdate();
        cv_.wait_for(lock, updateDelay_, [this]() { return !running_ || restart_; });
    }
    onShutdown();
}

void Service::stop()
{
    std::unique_lock lock(mutex_);
    running_ = false;
    cv_.notify_all();
}

void Service::restart()
{
    std::unique_lock lock(mutex_);
    restart_ = true;
    cv_.notify_all();
}

bool Service::daemonize()
{
    pid_t pid = fork();
    if (pid < 0) { throw std::runtime_error("fork failed: "s + std::strerror(errno)); }
    if (pid > 0) { std::cout << pid << std::endl; return false; }

    umask(0);
    
    //open log file
    openlog("PrinterDataCollector", 0, LOG_DAEMON);
    syslog(LOG_INFO, "imposter is sus");
    
    pid_t sid = setsid();
    if (sid < 0) { throw std::runtime_error("setsid failed: "s + std::strerror(errno)); }
    if (chdir("/") < 0) { throw std::runtime_error("chdir failed: "s + std::strerror(errno)); }

    std::signal(SIGTERM, signalHandler);
    std::signal(SIGHUP, signalHandler);

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    return true;
}

void Service::signalHandler(int sig)
{
    if (!serviceInstance_) { return; }
    switch (sig) 
    {
    case SIGTERM: serviceInstance_->stop(); break;
    case SIGHUP: serviceInstance_->restart(); break;
    }
}
