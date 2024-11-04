#include "SnmpPrinter.h"
#include <snmp_pp/vb.h>
#include <snmp_pp/pdu.h>
#include "SnmpException.h"

#define OID_PRINTER_MIB_ROOT "1.3.6.1.2.1.43"


SnmpPrinter::SnmpPrinter(const Snmp_pp::Address& address, const Snmp_pp::OctetStr& comunityName)
    : target_(std::make_unique<Snmp_pp::CTarget>(address, comunityName, comunityName))
{}

SnmpPrinter::SnmpPrinter(const Snmp_pp::Address& address, const Snmp_pp::OctetStr& userName, int securityModel)
    : target_(std::make_unique<Snmp_pp::UTarget>(address, userName, securityModel))
{}

SnmpPrinter::SnmpPrinter(std::unique_ptr<Snmp_pp::SnmpTarget> target)
    : target_(std::move(target))
{}

FieldList::ValueType convert(const Snmp_pp::Vb& vb)
{
    switch (vb.get_syntax())
    {
    case sNMP_SYNTAX_INT32:
    {
        int32_t val; vb.get_value(val);
        return val;
    }
    case sNMP_SYNTAX_TIMETICKS:
    {
        uint32_t val; vb.get_value(val);
        return (uint64_t)val;
    }
    case sNMP_SYNTAX_CNTR32:
    {
        uint32_t val; vb.get_value(val);
        return (uint64_t)val;
    }
    case sNMP_SYNTAX_GAUGE32:
    {
        uint32_t val; vb.get_value(val);
        return (uint64_t)val;
    }
    case sNMP_SYNTAX_CNTR64:
    {
        uint64_t val; vb.get_value(val);
        return val;
    }
    case sNMP_SYNTAX_BITS:
    case sNMP_SYNTAX_OCTETS:
    case sNMP_SYNTAX_IPADDR:
    case sNMP_SYNTAX_OID:
        return vb.get_printable_value();
    }
    return "null";
}

FieldList SnmpPrinter::getFullInfo(Snmp_pp::Snmp& session) const
{
    FieldList data;
    Snmp_pp::Oid end(OID_PRINTER_MIB_ROOT);
    end[end.len() - 1]++;
    Snmp_pp::Vb vb(OID_PRINTER_MIB_ROOT);
    Snmp_pp::Pdu pdu(&vb, 1);
    
    if (target_->get_type() == Snmp_pp::SnmpTarget::target_type::type_utarget) 
    { 
        pdu.set_security_level(0); 
    }

    while (true)
    {
        SNMP_CHECK(session.get_next(pdu, *target_));
        pdu.get_vb(vb, 0);
        if (vb.get_oid() < end) { break; }
        data.add(vb.get_printable_oid(), convert(vb));
    }

    return data;
}
