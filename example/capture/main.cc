#include <iostream>
#include <opencv2/opencv.hpp>

int main(int argc, char** argv)
{

    cv::Mat frame;
    cv::VideoCapture cap(cv::CAP_ANY);
    cap.set(cv::CAP_PROP_FORMAT, CV_8UC3);
    cap.set(cv::CAP_PROP_FRAME_WIDTH,1280);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT,960);


    if(!cap.isOpened())
    {
        std::cout << "Could not initialize capturing...\n";
        return 0;
    }


    int i = 0;
    while(1){
        i++;

        cap >> frame;
        std::cout << frame.rows << '\n';
        std::cout << frame.cols << '\n';

        if(frame.empty()) break; // end of video stream
        if(i==5) break;
    }
    cv::imwrite("./last_frame.jpg", frame);
}
