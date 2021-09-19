#include <iostream>
#include <vector>

#include "opencv2/opencv.hpp"
#include "mobilenetv2/mobilenetv2.h"
#include "timer.h"


int main(){
    if (argc != 4) {
        std::cerr << "Usage:\n\t./classify <model path> <repeat time> <img>\n";
        return 1;
    }

    std::string model_path(argv[1]);
    std::string repeat(argv[2]);
    std::string img_path(argv[3]);

    MobileNetv2 net(model_path);
    cv::Mat img = cv::imread(img_path);

    for(int i=0; i<std::stoi(repeat); i++){
        std::vector<float> result;
        Timer t;
        result = net.invoke(img);
    }
    return 0;
}
