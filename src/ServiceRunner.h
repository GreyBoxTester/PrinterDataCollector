#pragma once
#include <atomic>
#include <syslog.h>
#include "Service.h"

using namespace std::string_literals;

class ServiceRunner
{
public:
    static ServiceRunner& getInstance();

    template<typename S>
    void run() 
    {
        if (!daemonize()) { return; } 

        //open log file
        openlog("PrinterDataCollector", 0, LOG_DAEMON);
        syslog(LOG_INFO, "service starting up");

        S service;
        service_ = &service;
        service.run();
    }

private:
    static bool daemonize();
    static void signalHandler(int sig);
private:
    ServiceRunner() = default;
    ServiceRunner(const ServiceRunner&) = delete;
    ServiceRunner& operator=(const ServiceRunner&) = delete;
private:
    std::atomic<Service*> service_ = nullptr;
};
