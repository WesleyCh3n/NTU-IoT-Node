#include "opencv2/opencv.hpp"
using namespace std;

int main(){
    cv::VideoCapture camera(0);
    cv::Mat frame;
    if(!camera.open(0))
        return 1;
    camera.set(cv::CAP_PROP_FORMAT, CV_8UC3);
    camera.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    camera.set(cv::CAP_PROP_FRAME_HEIGHT, 960);
    for(int i=0; i<100; i++){
        camera >> frame;
    }
    std::cout << "finish" << '\n';
    cv::flip(frame, frame, -1);
    cv::imwrite("./frame.jpg", frame);
}

