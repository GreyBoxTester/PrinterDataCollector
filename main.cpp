#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <variant>
#include <memory>

#include <snmp_pp/snmp_pp.h>


#define SNMP_CHECK(expr) do { \
    int res = (expr); \
    if (res) { throw SnmpException(res); } \
} while (false) \


#define OID_PRINTER_MIB_ROOT "1.3.6.1.2.1.43"

/*#define OID_DEVICE_DESCRIPTION "1.3.6.1.2.1.1.1.0"

#define OID_PRINTER_SERIAL_NUMBER "1.3.6.1.2.1.43.5.1.1.17.1"
#define OID_LIFE_COUNT "1.3.6.1.2.1.43.10.2.1.4.1.1"

#define OID_SEQ_CONSUMABLES_NAME "1.3.6.1.2.1.43.11.1.1.6.1"
#define OID_SEQ_CONSUMABLES_VALUE_MAX "1.3.6.1.2.1.43.11.1.1.8.1"
#define OID_SEQ_CONSUMABLES_VALUE "1.3.6.1.2.1.43.11.1.1.9.1"

#define OID_SEQ_ALERTS_SEVERITY_LEVEL "1.3.6.1.2.1.43.18.1.1.2.1"
#define OID_SEQ_ALERTS_CODE "1.3.6.1.2.1.43.18.1.1.7.1"
#define OID_SEQ_ALERTS_DESCRIPTION "1.3.6.1.2.1.43.18.1.1.8.1"
#define OID_SEQ_ALERTS_TIME "1.3.6.1.2.1.43.18.1.1.9.1"*/


using namespace Snmp_pp;


class SnmpException : public std::exception
{
public:
    SnmpException(int errorCode) : errorCode_(errorCode) {}
    int getErrorCode() const noexcept { return errorCode_; }
    virtual const char* what() const noexcept override { return Snmp::error_msg(errorCode_); }
private:
    int errorCode_ = 0;
};


bool isInSubtree(const Oid& oid1, const Oid& oid2)
{
    if (oid1.len() > oid2.len()) { return false; }
    for (unsigned int i = 0; i < oid1.len(); i++)
    {
        if (oid1[i] != oid2[i]) { return false; }
    }
    return true;
}

std::string getAsString(Snmp& session, SnmpTarget& target, int securityLevel, const Oid& oid)
{
    Vb vb(oid);
    Pdu pdu(&vb, 1);
    pdu.set_security_level(securityLevel);

    SNMP_CHECK(session.get(pdu, target));
    pdu.get_vb(vb, 0);
    return vb.get_printable_value();
}

std::vector<Vb> walkSubtree(Snmp& session, SnmpTarget& target, int securityLevel, const Oid& root)
{
    std::vector<Vb> res;
    Vb vb(root);
    Pdu pdu(&vb, 1);
    pdu.set_security_level(securityLevel);

    while (true)
    {
        SNMP_CHECK(session.get_next(pdu, target));
        pdu.get_vb(vb, 0);
        if (!isInSubtree(root, vb.get_oid())) { break; }
        res.push_back(vb);
    }

    return res;
}

v3MP initV3MP(const char* bootCounterFile)
{
    OctetStr engineID = "PrinterDataCollector";
    unsigned int counter = 0;
    int status;
    status = getBootCounter(bootCounterFile, engineID, counter);
    if (status != SNMPv3_OK && status < SNMPv3_FILEOPEN_ERROR) { SNMP_CHECK(status); }
    counter++;
    SNMP_CHECK(saveBootCounter(bootCounterFile, engineID, counter));

    v3MP v3mp(engineID, counter, status);
    if (status != SNMPv3_MP_OK) { SNMP_CHECK(status); }

    return v3mp;
}

/*void printPrinterInfo(Snmp& session, SnmpTarget& target)
{
    std::cout << "Printer info:\n";
    std::cout << "  description: " << getAsString(session, target, OID_DEVICE_DESCRIPTION) << '\n';
    std::cout << "  serial number: " << getAsString(session, target, OID_PRINTER_SERIAL_NUMBER) << '\n';

    {
        auto names = walkSubtree(session, target, OID_SEQ_CONSUMABLES_NAME);
        auto values = walkSubtree(session, target, OID_SEQ_CONSUMABLES_VALUE);
        auto valuesMax = walkSubtree(session, target, OID_SEQ_CONSUMABLES_VALUE_MAX);

        std::cout << "cartriges:\n";
        for (size_t i = 0; i < names.size(); i++)
        {
            std::cout << "  " << names[i].get_printable_value() << " : " << values[i].get_printable_value() << " out of " << valuesMax[i].get_printable_value() << '\n';
        }
    }

    std::cout << "life count(pages or other unit): " << getAsString(session, target, OID_LIFE_COUNT) << '\n';

    {
        auto levels = walkSubtree(session, target, OID_SEQ_ALERTS_SEVERITY_LEVEL);
        auto codes = walkSubtree(session, target, OID_SEQ_ALERTS_CODE);
        auto descriptions = walkSubtree(session, target, OID_SEQ_ALERTS_DESCRIPTION);
        auto times = walkSubtree(session, target, OID_SEQ_ALERTS_TIME);

        std::cout << "alerts:\n";
        for (size_t i = 0; i < levels.size(); i++)
        {
            std::cout << "  alert " << i << ":\n";
            std::cout << "    severity: " << levels[i].get_printable_value() << '\n';
            std::cout << "    code: " << codes[i].get_printable_value() << '\n';
            std::cout << "    description: " << descriptions[i].get_printable_value() << '\n';
            std::cout << "    time: " << times[i].get_printable_value() << '\n';
        }
    }

    std::cout.flush();
}*/

int main()
{
    DefaultLog::log()->set_profile("off");

    Snmp::socket_startup();

    try
    {
        int status = 0;
        Snmp session(status);
        SNMP_CHECK(status);

        std::cout << "Enter ip address: ";
        std::string ip; std::cin >> ip;
        std::cout << "Use user target? [y/n]: ";
        char c; std::cin >> c; c = std::tolower(c);

        std::unique_ptr<SnmpTarget> target;
        v3MP v3mp = initV3MP("snmpv3_boot_counter");
        session.set_mpv3(&v3mp);

        int securityLevel = 0;

        if (c != 'y')
        {
            std::cout << "Enter community name: ";
            std::string communityName; std::cin >> communityName;
            target = std::make_unique<CTarget>(UdpAddress(ip.c_str()), communityName.c_str(), communityName.c_str());
        }
        else
        {
            std::cout << "Enter username: ";
            std::string username; std::cin >> username;

            std::cout << "Enter security level (noAuthNoPriv/authNoPriv/authPriv) [1-3]: ";
            std::cin >> securityLevel;
            if (securityLevel < 1 || securityLevel > 3) { throw std::runtime_error("bad security level"); }
            std::string authPwd, privPwd;
            int authProtocol = SNMP_AUTHPROTOCOL_NONE, privProtocol = SNMP_PRIVPROTOCOL_NONE;
            if (securityLevel > 1)
            {
                std::cout << "Enter auth protocol [MD5/SHA]: ";
                std::string authProtoName; std::cin >> authProtoName;
                if (authProtoName == "MD5") { authProtocol = SNMP_AUTHPROTOCOL_HMACMD5; }
                else if (authProtoName == "SHA") { authProtocol = SNMP_AUTHPROTOCOL_HMACSHA; }

                std::cout << "Enter auth pwd: ";
                std::cin >> authPwd;
            }
            if (securityLevel > 2)
            {
                std::cout << "Enter priv protocol [DES/3DESEDE/AES128/AES192/AES256]: ";
                std::string privProtoName; std::cin >> privProtoName;
                if (privProtoName == "DES") { privProtocol = SNMP_PRIVPROTOCOL_DES; }
                else if (privProtoName == "3DESEDE") { privProtocol = SNMP_PRIVPROTOCOL_3DESEDE; }
                else if (privProtoName == "AES128") { privProtocol = SNMP_PRIVPROTOCOL_AES128; }
                else if (privProtoName == "AES192") { privProtocol = SNMP_PRIVPROTOCOL_AES192; }
                else if (privProtoName == "AES256") { privProtocol = SNMP_PRIVPROTOCOL_AES256; }

                std::cout << "Enter priv pwd: ";
                std::cin >> privPwd;
            }

            v3mp.get_usm()->add_usm_user(username.c_str(), authProtocol, privProtocol, authPwd.c_str(), privPwd.c_str());

            target = std::make_unique<UTarget>(UdpAddress(ip.c_str()), username.c_str(), SNMP_SECURITY_MODEL_USM);
        }

        //printPrinterInfo(session, *target);

        auto printerInfo = walkSubtree(session, *target, securityLevel, OID_PRINTER_MIB_ROOT);
        for (auto& vb : printerInfo) 
        { 
            std::cout << vb.get_printable_oid() << ' ' << vb.get_printable_value() << std::endl;
        }
 
    }
    catch (const SnmpException& e)
    {
        std::cerr << e.what() << " (error code " << e.getErrorCode() << ')' << std::endl;
        Snmp::socket_cleanup();
        return -1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "std::exception: " << e.what() << std::endl;
        Snmp::socket_cleanup();
        return -1;
    }

    Snmp::socket_cleanup();
    return 0;
}
