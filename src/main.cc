#include <iostream>
#include <string>
#include <map>

#include "cow_monitor/cow_monitor.h"
#include "cxxopts.hpp"
using namespace std;


int main(int argc, char** argv){
    cxxopts::Options options("ntu-node", "Cow face monitoring system");
    options.add_options()
        ("s,stream", "Start streaming")
        ("n,node", "node number", cxxopts::value<std::string>()->default_value(""))
        ("i,image", "image directory to recognize", cxxopts::value<std::string>()->default_value(""))
        ("D,detect", "Detect model path", cxxopts::value<std::string>()->default_value("./model/yolov4-tiny-f16.tflite"))
        ("C,classify", "Classify model path", cxxopts::value<std::string>()->default_value("./model/mobilenetv2.tflite"))
        ("R,ref", "class reference", cxxopts::value<std::string>()->default_value("./cfg/ref.8f.tsv"))
        ("d,ref_dict", "class id", cxxopts::value<std::string>()->default_value("./cfg/ref_dict.csv"))
        ("f,fence", "fence bbox", cxxopts::value<std::string>()->default_value(""))
        ("q,mqtt_ip", "mqtt broker ip", cxxopts::value<std::string>()->default_value(""))
        ("u,user", "mqtt username", cxxopts::value<std::string>()->default_value(""))
        ("p,pwd", "mqtt password", cxxopts::value<std::string>()->default_value(""))
        ("v,version", "ntu-node version")
        ("h,help", "Print usage");
    auto result = options.parse(argc, argv);
    if (result.count("help")){
        std::cout << options.help() << std::endl;
        exit(0);
    }

    std::map<std::string, std::string> conf_map = {
        {"node", result["node"].as<std::string>()},
        {"detect", result["detect"].as<std::string>()},
        {"classify", result["classify"].as<std::string>()},
        {"ref", result["ref"].as<std::string>()},
        {"ref_dict", result["ref_dict"].as<std::string>()},
        {"fence", result["fence"].as<std::string>()},
        {"mqtt_ip", result["mqtt_ip"].as<std::string>()},
        {"user", result["user"].as<std::string>()},
        {"pwd", result["pwd"].as<std::string>()}
    };

    if(result.count("version")){
        const char *VERSION = NTU_IOT_NODE_VERSION;
        std::cout << "Version " << VERSION << '\n';
        return 0;
    }

    if(result.count("stream")){
        std::cout<< u8"\033[2J\033[1;1H";
        CowMonitor cow_monitor;
        if(!cow_monitor.Init(conf_map)){
            cerr << "Cow Monitor Init Failed! Stop!\n";
            cerr << "\tCheck cow ref or fence cfg are given properly\n";
            exit(-1);
        }
        if(!cow_monitor.Stream())
            cerr << "Cow Monitor Streaming Failed! Stop!\n";
    }

    if(result.count("image")){
        std::cout<< u8"\033[2J\033[1;1H";
        CowMonitor cow_monitor;
        if(!cow_monitor.Init(conf_map)){
            cerr << "Cow Monitor Init Failed! Stop!\n";
            cerr << "\tCheck cow ref or fence cfg are given properly\n";
            exit(-1);
        }
        std::cout << "Recognize image\n";
        cow_monitor.RunImage(result["image"].as<std::string>());
    }

    return 0;
}
