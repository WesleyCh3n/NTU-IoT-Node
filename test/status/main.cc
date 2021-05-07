#include <iostream>
#include <array>
#include <ctime>
#include <sstream>
#include "mqtt/client.h"


int main(int argc, char* argv[])
{
    std::string ID = argv[1];
    std::string cliID = "node"+ID;
    std::cout << cliID << " sending status..." << '\n';

    mqtt::client cli("tcp://140.112.94.123:1883", cliID);
    std::string user = "wesley";
    std::string password = "ntubime405";
    mqtt::connect_options connOpts;
    connOpts.set_keep_alive_interval(20);
    connOpts.set_clean_session(true);
    connOpts.set_user_name(user);
    connOpts.set_password(password);

    std::stringstream MSG;
    MSG << "NODE,farm=NTU,monitor=feed " << cliID << "=1\n";
    std::cout << MSG.str();
    try {
        // Connect to the client
        cli.connect(connOpts);
        auto msg = mqtt::make_message("sensors", MSG.str());
        msg->set_qos(1);
        cli.publish(msg);
        cli.disconnect();
        std::cout << "message sent!\n";
    }
    catch (const mqtt::exception& exc) {
        std::cerr << "Error: " << exc.what() << " ["
            << exc.get_reason_code() << "]" << std::endl;
        return 1;
    }

    return 0;
}
