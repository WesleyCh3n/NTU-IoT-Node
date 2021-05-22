#include <iostream>
#include <vector>

#include "opencv2/opencv.hpp"
#include "yolov4-tiny.h"
#include "../src/timer.h"


int main(){
    Yolov4Tiny net("../model/yolov4-tiny-f16.tflite", 0.8, 0.6, 1280, 960);
    cv::Mat img = cv::imread("./detect.jpg");
    for(int i=0; i<10; i++){
        std::vector<cv::Rect> result;
        Timer t;
        net.invoke(img, result);
    }
    return 0;
}
