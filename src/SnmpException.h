#pragma once
#include <snmp_pp/uxsnmp.h>

#define SNMP_CHECK(expr) do { \
    int res = (expr); \
    if (res) { throw SnmpException(res); } \
} while (false) \


class SnmpException : public std::exception
{
public:
    SnmpException(int errorCode) : errorCode_(errorCode) {}
    int getErrorCode() const noexcept { return errorCode_; }
    virtual const char* what() const noexcept override { return Snmp_pp::Snmp::error_msg(errorCode_); }
private:
    int errorCode_ = 0;
};
