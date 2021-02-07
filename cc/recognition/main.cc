#include <iostream>
#include <string>
#include "include/cow_monitor.h"
using namespace std;

int main(){
    if (system("CLS")) system("clear");
    cm::CowMonitor cow_monitor;
    string model_path[1] = {"./model/yolov4-tiny-416-fp16.tflite"};
    if(!cow_monitor.Init(model_path, cm::DETECT))
        cerr << "Stop!\n";
    if(!cow_monitor.Stream()) cerr << "Stop!\n";
    return 0;
}
