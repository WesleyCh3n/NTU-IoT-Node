#include <iostream>
#include <string>

#include "include/cow_monitor.h"
#include "cxxopts.hpp"
using namespace std;


int main(int argc, char** argv){
    cxxopts::Options options("ntu-iot-node", "Cow face monitoring system");
    options.add_options()
        ("s,stream", "Start streaming")
        ("n,node", "node number", cxxopts::value<std::string>()->default_value(""))
        ("i,image", "Recognize Image", cxxopts::value<std::string>())
        ("m,mode", "mode: 0:detect 1:classify 2:recognize", cxxopts::value<int>()->default_value("2"))
        ("D,detect", "Detect model path", cxxopts::value<std::string>()->default_value("./model/yolov4-tiny-416-fp16.tflite"))
        ("C,classify", "Classify model path", cxxopts::value<std::string>()->default_value("./model/mobilenetv2.tflite"))
        ("R,ref", "class reference", cxxopts::value<std::string>()->default_value("./cfg/ref.8f.tsv"))
        ("d,dict", "class id", cxxopts::value<std::string>()->default_value("./cfg/ref_dict.csv"))
        ("f,fence", "fence bbox", cxxopts::value<std::string>()->default_value("./cfg/node02_fence.csv"))
        ("q,mqtt", "mqtt address", cxxopts::value<std::string>())
        ("u,user", "mqtt username", cxxopts::value<std::string>())
        ("p,pwd", "mqtt password", cxxopts::value<std::string>())
        ("v,version", "ntu-iot-node version")
        ("h,help", "Print usage");
    auto result = options.parse(argc, argv);
    if (result.count("help")){
        std::cout << options.help() << std::endl;
        exit(0);
    }
    /* init model variable */
    std::string model_path[2] = {
        result["detect"].as<std::string>(),
        result["classify"].as<std::string>()
    };
    if(result.count("version")){
        const char *VERSION = NTU_IOT_NODE_VERSION;
        std::cout << "Version " << VERSION << '\n';
        return 0;
    }

    if(result.count("stream") && result.count("image")){
        std::cout << "Error:\n"
                  << "\tYou are not suppose to stream & run image at the same time\n";
        exit(-1);
    }
    if(result.count("stream")){
        std::cout<< u8"\033[2J\033[1;1H";
        cm::CowMonitor cow_monitor;
        if(result.count("mqtt"))
            cow_monitor.InitMqtt(result["mqtt"].as<std::string>(),
                                 result["user"].as<std::string>(),
                                 result["pwd"].as<std::string>());
        if(!cow_monitor.Init(result["node"].as<std::string>(),
                             model_path,
                             result["ref"].as<std::string>(),
                             result["dict"].as<std::string>(),
                             result["fence"].as<std::string>(),
                             result["mode"].as<int>())){
            cerr << "Stop!\n";
            return -1;
        }
        if(!cow_monitor.Stream()) cerr << "Stop!\n";
    }
    if(result.count("image")){
        std::cout<< u8"\033[2J\033[1;1H";
        cm::CowMonitor cow_monitor;
        if(!cow_monitor.Init(result["node"].as<std::string>(),
                             model_path,
                             result["ref"].as<std::string>(),
                             result["dict"].as<std::string>(),
                             result["fence"].as<std::string>(),
                             result["mode"].as<int>())){
            cerr << "Stop!\n";
            return -1;
        }
        std::cout << "Recognize image\n";
        cow_monitor.RunImage(result["image"].as<std::string>());
    }

    return 0;
}
