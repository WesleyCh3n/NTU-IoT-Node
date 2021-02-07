#include <iostream>

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <vector>

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"

#include "boost/format.hpp"
#include "opencv2/opencv.hpp"
#include "raspicam/raspicam_cv.h"
#include "NumCpp.hpp"
#include "include/cow_monitor.h"

using namespace std;
using namespace cm;


/*=================== CowMonitor Class definition ===================*/

auto CowMonitor::Init(std::string model_path[], int MODE) -> bool{
    cout << "MODE: " << MODE << '\n';
    switch(MODE){
        case cm::DETECT:{
            bool d_status = initdModel(model_path[0]);
            return d_status;
            break;
        }
        case cm::CLASSIFY:
            break;
        case cm::RECOGNIZE:
            break;
    }
    return false;
}

auto CowMonitor::initdModel(std::string model_path) -> bool{
    printf("Initialize Detection Model\n");
    d_model_= tflite::FlatBufferModel::BuildFromFile(model_path.c_str());
    tflite::ops::builtin::BuiltinOpResolver d_resolver;
    tflite::InterpreterBuilder(*d_model_.get(), d_resolver)(&d_interpreter_);
    d_interpreter_->AllocateTensors();

    d_input_tensor_ = d_interpreter_->tensor(d_interpreter_->inputs()[0]);
    d_output_box_ = d_interpreter_->tensor(d_interpreter_->outputs()[0]);
    d_output_score_ = d_interpreter_->tensor(d_interpreter_->outputs()[1]);

    d_input_dim_.height = d_input_tensor_->dims->data[1];
    d_input_dim_.width = d_input_tensor_->dims->data[2];
    d_input_dim_.channel = d_input_tensor_->dims->data[3];
    return true;
}

auto CowMonitor::matPreprocess(cv::Mat &src, uint width, uint height) -> cv::Mat{
    // convert to float; BGR -> RGB
    cv::Mat dst;
    src.convertTo(dst, CV_32FC3);
    cv::cvtColor(dst, dst, cv::COLOR_BGR2RGB);

    // normalize
    Pixel* pixel = dst.ptr<Pixel>(0,0);
    const Pixel* endPixel = pixel + dst.cols * dst.rows;
    for (; pixel != endPixel; pixel++)
        cm::model::yolov4::norm(*pixel);

    // resize image as model input
    cv::resize(dst, dst, cv::Size(width, height));
    return dst;
}

auto CowMonitor::yoloResult(vector<float> &box,
                vector<float> &score,
                float thres,
                vector<cv::Rect> &result) -> bool{

    auto it = std::find_if(std::begin(score), std::end(score),
                           [&thres](float i){return i > thres;});
    vector<cv::Rect> rects;
    vector<float> scores;
    while (it != std::end(score)) {
        size_t id = std::distance(std::begin(score), it);
        const int cx = box[4*id];
        const int cy = box[4*id+1];
        const int w = box[4*id+2];
        const int h = box[4*id+3];
        const int xmin = ((cx-(w/2.f))/d_input_dim_.width)*vW_;
        const int ymin = ((cy-(h/2.f))/d_input_dim_.height)*vH_;
        const int xmax = ((cx+(w/2.f))/d_input_dim_.width)*vW_;
        const int ymax = ((cy+(h/2.f))/d_input_dim_.height)*vH_;
        rects.emplace_back(cv::Rect(xmin, ymin, xmax-xmin, ymax-ymin));
        scores.emplace_back(score[id]);
        it = std::find_if(std::next(it), std::end(score),
                          [&thres](float i){return i > thres;});
    }
    if(rects.empty())
        return false;

    vector<int> ids;
    cv::dnn::NMSBoxes(rects, scores, thres, 0.4, ids);
    if(ids.empty())
        return false;
    for(int tmp: ids){
        result.emplace_back(rects[tmp]);
    }
    return true;
}

auto CowMonitor::Detection(cv::Mat inputImg, std::vector<cv::Rect> &result_box) -> bool{
    inputImg = matPreprocess(inputImg, d_input_dim_.width, d_input_dim_.height);
    // flatten rgb image to input layer.
    memcpy(d_input_tensor_->data.f, inputImg.ptr<float>(0),
           d_input_dim_.height * d_input_dim_.height *
           d_input_dim_.channel * sizeof(float));
    d_interpreter_->Invoke();
    vector<float> box_vec = cm::model::cvtTensor(d_output_box_);
    vector<float> score_vec = cm::model::cvtTensor(d_output_score_);
    return yoloResult(box_vec, score_vec, 0.6, result_box);
}

auto CowMonitor::Stream(int width, int height) -> bool{
    std::chrono::time_point<std::chrono::system_clock> start, end;
    std::chrono::duration<double> elapsed_seconds;
    raspicam::RaspiCam_Cv Camera;
    cv::Mat frame;
    Camera.set(cv::CAP_PROP_FORMAT, CV_8UC3);
    Camera.set(cv::CAP_PROP_FRAME_WIDTH, width);
    Camera.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    vW_ = width; vH_ = height;
    cout << "Opening Camera...\n";
    if(!Camera.open()){
        cerr << "Error opening the camera\n";
        return -1;
    }
    for(;;){
        // start = std::chrono::system_clock::now();
        time_t now = time(0);
        tm *ltm = localtime(&now);
        std::string YMD = boost::str(boost::format("%04d_%02d_%02d")%
                                     (1900+ltm->tm_year)%
                                     (1+ltm->tm_mon)%
                                     (ltm->tm_mday));
        std::string HMS = boost::str(boost::format("%02d_%02d_%02d")%
                                     (ltm->tm_hour)%
                                     (ltm->tm_min)%
                                     (ltm->tm_sec));
        std::string img_dir = "/home/data/img/"+YMD+"/";
#ifdef RELEASE
        std::filesystem::create_directories(img_dir);
        std::ofstream csvFile("/home/data/"+YMD+"-detect.csv", std::ios::app);
        if(!csvFile.is_open()) cerr << "open csv failed\n";
#endif

        printf("\r%s-%s", YMD.c_str(), HMS.c_str());
        fflush(stdout);
        Camera.grab();
        Camera.retrieve(frame);
        vector<cv::Rect> result_box;
        if(Detection(frame, result_box)){
            printf("\tThere are %d\n", result_box.size());
            fflush(stdout);
#ifdef RELEASE
            std::filesystem::current_path(img_dir);
            cv::imwrite(YMD+"-"+HMS+".jpg", frame);
            csvFile << YMD+"-"+HMS+".jpg"<< ","
                    << std::to_string(result_box.size()) << '\n';
            csvFile.flush();
#endif
        }
        else{
#ifdef RELEASE
            std::filesystem::current_path("/home/data/");
#endif
            printf("\tNothing");
            fflush(stdout);
        }

        // end = std::chrono::system_clock::now();
        // elapsed_seconds = end - start;
        // printf("\rs: %.5f", elapsed_seconds.count());
    }
    Camera.release();
    // cv::flip(frame, frame, -1);
    // cv::imwrite("raspicam_cv_image.jpg", frame);
    return true;
}

auto CowMonitor::RunImage(std::string fileName){
}


/*=================== Other namespace definition ===================*/

auto cm::model::cvtTensor(TfLiteTensor* tensor) -> std::vector<float>{
    int nelem = 1;
    for(int i=0; i<tensor->dims->size; ++i) //length of dim. ex: (1,4,4) -> 3
        nelem *= tensor->dims->data[i]; //dim number. ex: (1,3,3) -> nelem=9
    vector<float> data(tensor->data.f, tensor->data.f+nelem);
    return data;
}

void cm::model::yolov4::norm(Pixel &pixel){
    pixel.x = (pixel.x / 255.0);
    pixel.y = (pixel.y / 255.0);
    pixel.z = (pixel.z / 255.0);
}

void cm::model::mobilenetv2::norm(Pixel &pixel){
    pixel.x = ((pixel.x / 255.0)-0.5)*2.0;
    pixel.y = ((pixel.y / 255.0)-0.5)*2.0;
    pixel.z = ((pixel.z / 255.0)-0.5)*2.0;
}
