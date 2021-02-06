#include <iostream>
#include "include/cow_monitor.h"
using namespace std;

int main(){
    if (system("CLS")) system("clear");
    CowMonitor cow_monitor;
    if(!cow_monitor.Init("./model/yolov4-tiny-416-fp16.tflite"))
        cerr << "Stop!\n";
    if(!cow_monitor.Stream()) cerr << "Stop!\n";
    return 0;
}
