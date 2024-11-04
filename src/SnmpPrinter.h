#pragma once
#include <memory>
#include <snmp_pp/uxsnmp.h>
#include <snmp_pp/target.h>
#include "FieldList.h"


class SnmpPrinter
{
public:
    SnmpPrinter(const Snmp_pp::Address& address, const Snmp_pp::OctetStr& communityName = "public");
    SnmpPrinter(const Snmp_pp::Address& address, const Snmp_pp::OctetStr& userName, int securityModel);
    SnmpPrinter(std::unique_ptr<Snmp_pp::SnmpTarget> target);
    FieldList getFullInfo(Snmp_pp::Snmp& session) const;
private:
    std::unique_ptr<Snmp_pp::SnmpTarget> target_;
};
