#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <variant>
#include <memory>
#include <syslog.h>

#include "SnmpPtrinter.h"
#include "SnmpException.h"
#include "Service.h"

#include <snmp_pp/snmp_pp.h>
using namespace Snmp_pp;

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

using namespace std::chrono_literals;

class DummyService : public Service
{
public:
    DummyService()
        : Service(1s)
    {}
protected:
    virtual void onStartup() override 
    {
        syslog(LOG_INFO, "startup");
    }

    virtual void onUpdate() override 
    {
        //iter++;
        ///if (iter > 5) { stop(); }
        syslog(LOG_ALERT, "sussy baka");
    }

    virtual void onShutdown() override 
    {
        syslog(LOG_INFO, "shutdown");
    }

    virtual void onRestart() override 
    {
        syslog(LOG_INFO, "restart");
    }

private:
    int iter = 0;
};

int main()
{
    DummyService().run();

    /*DefaultLog::log()->set_profile("off");

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

        SnmpPrinter printer(std::move(target));
        std::cout << printer.getFullInfo(session).toJson();
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
    return 0;*/
}
