#include <iostream>
#include <string>

#include "include/cow_monitor.h"
#include "cxxopts.hpp"
using namespace std;

int main(int argc, char** argv){
    cxxopts::Options options("ntu-iot-node", "Cow face monitoring system");
    options.add_options()
        ("s,stream", "Start streaming")
        ("i,image", "Recognize Image", cxxopts::value<std::string>())
        ("m,mode", "mode: 0:detect 1:classify 2:recognize", cxxopts::value<int>()->default_value("2"))
        ("D,detect", "Detect model path", cxxopts::value<std::string>()->default_value("./model/yolov4-tiny-416-fp16.tflite"))
        ("C,classify", "Classify model path", cxxopts::value<std::string>()->default_value("./model/mobilenetv2-128.tflite"))
        ("R,ref", "class reference", cxxopts::value<std::string>()->default_value("./19-01.tsv"))
        ("h,help", "Print usage");
    auto result = options.parse(argc, argv);
    if (result.count("help")){
        std::cout << options.help() << std::endl;
        exit(0);
    }
    std::string model_path[2];
    model_path[0] = result["detect"].as<std::string>();
    model_path[1] = result["classify"].as<std::string>();
    std::string ref = result["ref"].as<std::string>();
    int mode = result["mode"].as<int>();

    if(result.count("stream") && result.count("image")){
        std::cout << "Error:\n"
                  << "\tYou are not suppose to stream & run image at the same time\n";
        exit(-1);
    }
    if(result.count("stream")){
        if(system("CLS")) system("clear");
        cm::CowMonitor cow_monitor;
        if(!cow_monitor.Init(model_path, ref, mode)){
            cerr << "Stop!\n";
            return -1;
        }
        if(!cow_monitor.Stream()) cerr << "Stop!\n";
    }
    if(result.count("image")){
        if (system("CLS")) system("clear");
        cm::CowMonitor cow_monitor;
        if(!cow_monitor.Init(model_path, ref, mode)){
            cerr << "Stop!\n";
            return -1;
        }
        std::cout << "Recognize image\n";
        cow_monitor.RunImage(result["image"].as<std::string>());
    }

    return 0;
}
