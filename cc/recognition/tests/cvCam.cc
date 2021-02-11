#include "opencv2/opencv.hpp"
using namespace std;

int main(){
    cv::VideoCapture cap(0);
    cv::Mat frame;
    if(!cap.open(0))
        return 1;
    for(int i=0; i<100; i++){
        cap >> frame;
    }
    std::cout << "finish" << '\n';
    cv::flip(frame, frame, -1);
    cv::imwrite("./frame.jpg", frame);
}

