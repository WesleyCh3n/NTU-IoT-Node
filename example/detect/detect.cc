#include <iostream>
#include <vector>

#include "opencv2/opencv.hpp"
#include "yolov4-tiny/yolov4-tiny.h"
#include "timer.h"


int main(int argc, char *argv[]){
    if (argc != 4) {
        std::cerr << "Usage:\n\t./detect <model path> <repeat time> <img>\n";
        return 1;
    }
    std::string model_path(argv[1]);
    std::string repeat(argv[2]);
    std::string img_path(argv[3]);

    Yolov4Tiny net(model_path, 0.5, 0.6, 1280, 960);
    cv::Mat img = cv::imread(img_path);

    std::vector<cv::Rect> result;
    for(int i=0; i<std::stoi(repeat); i++){
        result.clear();
        Timer t;
        net.invoke(img, result);
    }

    for(auto box: result){
        cv::rectangle(img, box, cv::Scalar(255,255,0), 2);
    }
    cv::imwrite("./result.jpg", img);
    return 0;
}
