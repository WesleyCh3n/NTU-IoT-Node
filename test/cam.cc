#include <ctime>
#include <iostream>
#include "raspicam/raspicam_cv.h"
#include "opencv2/opencv.hpp"
using namespace std;

int main(){
    raspicam::RaspiCam_Cv Camera;
    cv::Mat frame;
    Camera.set(cv::CAP_PROP_FORMAT, CV_8UC3);
    Camera.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    Camera.set(cv::CAP_PROP_FRAME_HEIGHT, 960);
    if (!Camera.open()) {cerr<<"Error opening the camera"<<endl;return -1;}
    for ( int i=0; i<100; i++ ) {
        Camera.grab();
        Camera.retrieve(frame);
        if ( i%5==0 )  std::cout<<"\r captured "<<i<<" images"<<std::flush;
    }
    std::cout << "finish" << '\n';
    Camera.release();
    cv::flip(frame, frame, -1);
    cv::imwrite("./frame.jpg", frame);
}
