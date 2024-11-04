#include "ServiceRunner.h"
#include <stdexcept>
#include <cstring>
#include <csignal>
#include <sys/stat.h>

ServiceRunner& ServiceRunner::getInstance()
{
    static ServiceRunner runner;
    return runner;
}

bool ServiceRunner::daemonize()
{
    pid_t pid = fork();
    if (pid < 0) { throw std::runtime_error("fork failed: "s + std::strerror(errno)); }
    if (pid > 0) { return false; }

    umask(0);
    
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

void ServiceRunner::signalHandler(int sig)
{
    if (!ServiceRunner::getInstance().service_.load()) { return; }
    switch (sig) 
    {
    case SIGTERM: ServiceRunner::getInstance().service_.load()->stop(); break;
    case SIGHUP: ServiceRunner::getInstance().service_.load()->reload(); break;
    }
}