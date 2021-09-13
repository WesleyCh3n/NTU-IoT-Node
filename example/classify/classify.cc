#include <iostream>
#include <vector>

#include "opencv2/opencv.hpp"
#include "mobilenetv2/mobilenetv2.h"
#include "timer.h"


int main(){
    MobileNetv2 net("../model/mobilenetv2.tflite");
    cv::Mat img = cv::imread("./classify.jpg");
    for(int i=0; i<10; i++){
        std::vector<float> result;
        Timer t;
        result = net.invoke(img);
    }
    return 0;
}
