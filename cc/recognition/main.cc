#include <iostream>
#include <string>
#include "include/cow_monitor.h"
using namespace std;

int main(){
    if (system("CLS")) system("clear");
    cm::CowMonitor cow_monitor;
    string model_path[2] = {"./model/yolov4-tiny-416-fp16.tflite",
                            "./model/mobilenetv2-128.tflite"};
    if(!cow_monitor.Init(model_path, "./19-01.tsv", cm::RECOGNIZE)){
        cerr << "Stop!\n";
        return -1;
    }
    // if(!cow_monitor.Stream()) cerr << "Stop!\n";
    cow_monitor.RunImage("./tests/NODE29.jpg");
    return 0;
}
