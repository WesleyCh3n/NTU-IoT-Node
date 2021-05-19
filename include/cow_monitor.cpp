#include "include/cow_monitor.h"

#include <iostream>

#include <algorithm>
#include <array>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"

#include "boost/format.hpp"
#include "boost/math/tools/norms.hpp"
#include "opencv2/opencv.hpp"
#include "raspicam/raspicam_cv.h"
#include "mqtt/client.h"
using namespace std;
using namespace cm;

const char *VERSION = NTU_IOT_NODE_VERSION;

class Timer{
    public:
        Timer(){
            stTimePt_ = std::chrono::high_resolution_clock::now();
        }
        ~Timer(){
            Stop();
        }
        void Stop(){
            auto endTimePt = std::chrono::high_resolution_clock::now();
            auto start = std::chrono::time_point_cast<std::chrono::microseconds>
                         (stTimePt_).time_since_epoch().count();
            auto end = std::chrono::time_point_cast<std::chrono::microseconds>
                         (endTimePt).time_since_epoch().count();
            auto duration = end - start;
            std::cout << "take " << duration << "us ("
                      << duration*0.001 << "ms)\n";
        }
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> stTimePt_;
};

/*=================== CowMonitor Class definition ===================*/

/*
 * Initialize mqtt config
 * @param
 *     [in] ip: up of mqtt server
 *     [in] user: username of mqtt server
 *     [in] pwd: password of mqtt server
 *  */
void CowMonitor::InitMqtt(std::string ip, std::string user, std::string pwd){
    ip_ = "tcp://" + ip;
    user_ = user;
    pwd_ = pwd;
    std::string cliID = "node" + node_;
    /* TODO: use this wierd ptr & static solve initilize mqtt client */
    static mqtt::client cli(ip_,cliID);
    cli_ = &cli;
    connOpts_.set_keep_alive_interval(20);
    connOpts_.set_clean_session(true);
    connOpts_.set_user_name(user_);
    connOpts_.set_password(pwd_);
}

/*
 * Pub func: Initialize parameter
 * @param
 *     [in] node: node number
 *     [in] model_path: list of model path. [detection, classification]
 *     [in] ref: reference vector of cow
 *     [in] mode: 0->detect,1->classify,2->recognize
 * @return
 *     Init Status
 *  */
bool CowMonitor::Init(std::string node, std::string model_path[],
                      std::string ref_path, std::string dict_path,
                      std::string fence_path, int mode){
    node_ = node;
    std::cout << std::left << "====================================\n"
              << std::setw(16) << "Version" << ": " << VERSION << '\n'
              << std::setw(16) << "Node" << ": " << node_ << '\n'
              << std::setw(16) << "Mode" << ": " << mode << '\n'
              << std::setw(16) << "Mqtt" << ": " << ip_ << '\n'
              << std::setw(16) << "Mqtt user" << ": " << user_ << '\n';
    initCowRefs(ref_path, dict_path);
    initFenceCfg(fence_path);
    switch(mode){
        case cm::DETECT:{
            bool d_status = initdModel(model_path[0]);
            return d_status;
            // break;
        }
        case cm::CLASSIFY:{
            bool s_status = initcModel(model_path[1]);
            return s_status;
            // break;
        }
        case cm::RECOGNIZE:{
            bool d_status = initdModel(model_path[0]);
            bool s_status = initcModel(model_path[1]);
            return (d_status && s_status);
            // break;
        }
    }
    return false;
}

void CowMonitor::initCowRefs(std::string ref_path, std::string dict_path) {
    std::vector< std::vector<float> > feats = cm::model::readTSV(ref_path);
    std::ifstream inFile(dict_path);
    std::string id;
    int i = 0;
    while(getline(inFile, id)){
        cowRefs_[i].id = std::stoi(id);
        cowRefs_[i].feat = feats[i];
        i++;
    }
}

void CowMonitor::initFenceCfg(std::string fence_path){
    std::vector< std::vector<int> > bboxes = cm::model::readCSV(fence_path);
    for(int i=0; i<bboxes.size(); i++){
        fences_[i].f_id = i;
        int x = bboxes[i][0];
        int y = bboxes[i][1];
        int w = bboxes[i][2] - x;
        int h = bboxes[i][3] - y;
        fences_[i].bbox = cv::Rect(x, y, w, h);
        // printf("%d: %d, %d, %d, %d\n", fences_[i].f_id,
        //         fences_[i].bbox.x, fences_[i].bbox.y,
        //         fences_[i].bbox.width, fences_[i].bbox.height);
    }
}

/*
 * Allocate detection model to tensor
 * @param
 *     [in] model_path: model path
 * @return
 *     status
 *  */
bool CowMonitor::initdModel(std::string model_path){
    d_model_= tflite::FlatBufferModel::BuildFromFile(model_path.c_str());
    tflite::ops::builtin::BuiltinOpResolver d_resolver;
    tflite::InterpreterBuilder(*d_model_.get(), d_resolver)(&d_interpreter_);
    d_interpreter_->AllocateTensors();

    d_input_tensor_ = d_interpreter_->tensor(d_interpreter_->inputs()[0]);
    d_output_box_ = d_interpreter_->tensor(d_interpreter_->outputs()[0]);
    d_output_score_ = d_interpreter_->tensor(d_interpreter_->outputs()[1]);

    d_input_dim_.h = d_input_tensor_->dims->data[1];
    d_input_dim_.w = d_input_tensor_->dims->data[2];
    d_input_dim_.c = d_input_tensor_->dims->data[3];
    std::cout << std::left
              << std::setw(16) << "Detect Model" << ": " << model_path << '\n'
              << std::setw(16) << "Input size" << ": " << d_input_dim_.h << " "
              << d_input_dim_.w << " " << d_input_dim_.c << '\n';
    return true;
}

/*
 * Allocate classification model to tensor
 * @param
 *     [in] model_path: model path
 * @return
 *     status
 *  */
bool CowMonitor::initcModel(std::string model_path){
    c_model_= tflite::FlatBufferModel::BuildFromFile(model_path.c_str());
    tflite::ops::builtin::BuiltinOpResolver c_resolver;
    tflite::InterpreterBuilder(*c_model_.get(), c_resolver)(&c_interpreter_);
    c_interpreter_->AllocateTensors();

    c_input_tensor_ = c_interpreter_->tensor(c_interpreter_->inputs()[0]);
    c_output_tensor_ = c_interpreter_->tensor(c_interpreter_->outputs()[0]);

    c_input_dim_.h = c_input_tensor_->dims->data[1];
    c_input_dim_.w = c_input_tensor_->dims->data[2];
    c_input_dim_.c = c_input_tensor_->dims->data[3];
    std::cout << std::left
              << std::setw(16) << "Classify Model" << ": " << model_path << '\n'
              << std::setw(16) << "Input size" << ": " << c_input_dim_.h << " "
              << c_input_dim_.w << " " << c_input_dim_.c << '\n';
    return true;
}

/*
 * Preprocess input cv::Mat image
 * @param
 *     [out] src: input image
 *     [in] width: model input width
 *     [in] height: model input height
 *     [in] *norm(pixel &): pointer of function of normalization take pixel as
 *                          reference input
 * @return
 *     preprocessed image
 *  */
auto CowMonitor::matPreprocess(cv::Mat &src, uint width, uint height,
                               void (*norm)(Pixel&)) -> cv::Mat{
    // convert to float; BGR -> RGB
    cv::Mat dst;
    src.convertTo(dst, CV_32FC3);
    cv::cvtColor(dst, dst, cv::COLOR_BGR2RGB);

    // normalize
    Pixel* pixel = dst.ptr<Pixel>(0,0);
    const Pixel* endPixel = pixel + dst.cols * dst.rows;
    for (; pixel != endPixel; pixel++)
        norm(*pixel);

    // resize image as model input
    cv::resize(dst, dst, cv::Size(width, height));
    return dst;
}

/*
 * Find valid bbox by confidence and non-maximum suppression
 * @param
 *     [in] box: raw result of yolo bbox
 *     [in] score: raw result of yolo score
 *     [out] result: valid bbox with cv::Rect(xmin,xmax,w,h) as vector
 * @return
 *     status: if no valid bbox return false else true
 *  */
auto CowMonitor::yoloResult(vector<float> &box,
                vector<float> &score,
                float thres,
                vector<cv::Rect> &result) -> bool{
    auto it = std::find_if(std::begin(score), std::end(score),
                           [&thres](float i){return i > thres;});
    vector<cv::Rect> rects;
    vector<float> scores;
    cv::Rect roiImg(0, 0, vW_, vH_);
    while (it != std::end(score)) {
        size_t id      = std::distance(std::begin(score), it);
        const int cx   = box[4*id];
        const int cy   = box[4*id+1];
        const int w    = box[4*id+2];
        const int h    = box[4*id+3];
        const int xmin = ((cx-(w/2.f))/d_input_dim_.w) * vW_;
        const int ymin = ((cy-(h/2.f))/d_input_dim_.h) * vH_;
        const int xmax = ((cx+(w/2.f))/d_input_dim_.w) * vW_;
        const int ymax = ((cy+(h/2.f))/d_input_dim_.h) * vH_;
        rects.emplace_back(cv::Rect(xmin, ymin, xmax-xmin, ymax-ymin) & roiImg);
        scores.emplace_back(score[id]);
        it = std::find_if(std::next(it), std::end(score),
                          [&thres](float i){return i > thres;});
    }
    if(rects.empty())
        return false;
    vector<int> ids;
    cv::dnn::NMSBoxes(rects, scores, thres, cm::model::yolov4::NMS_THRES, ids);
    if(ids.empty())
        return false;
    for(int &tmp: ids){
        result.emplace_back(rects[tmp]);
    }
    return true;
}

/*
 * Detection pipeline for yolo
 * @param
 *     [in] inputImg: input image
 *     [out] result_box: result bounding box of input image
 * @return
 *     status: if no valid bbox return false else true
 *  */
auto CowMonitor::detection(cv::Mat inputImg,
                           std::vector<cv::Rect> &result_box) -> bool{
    inputImg = matPreprocess(inputImg, d_input_dim_.w, d_input_dim_.h,
                             cm::model::yolov4::norm);
    memcpy(d_input_tensor_->data.f, inputImg.ptr<float>(0),
           d_input_dim_.h * d_input_dim_.h *
           d_input_dim_.c * sizeof(float));
    d_interpreter_->Invoke();
    vector<float> box_vec   = cm::model::cvtTensor(d_output_box_);
    vector<float> score_vec = cm::model::cvtTensor(d_output_score_);
    return yoloResult(box_vec, score_vec,
                      cm::model::yolov4::CON_THRES, result_box);
}

/*
 * Classification pipeline base on yolo result
 * @param
 *     [in] inputImg: input image
 *     [in] detect_box: detection result box
 *     [out] result: ids base on bbox
 *  */
auto CowMonitor::classification(cv::Mat inputImg) -> int{
#ifdef BENCHMARK
    Timer timer;
#endif
    // cv::Mat cropImg = inputImg(roi);
    inputImg = matPreprocess(inputImg,
            c_input_dim_.w, c_input_dim_.h,
            cm::model::mobilenetv2::norm);
    memcpy(c_input_tensor_->data.f, inputImg.ptr<float>(0),
            c_input_dim_.h * c_input_dim_.h *
            c_input_dim_.c * sizeof(float));
    {
#ifdef BENCHMARK
        Timer timer;
#endif
        c_interpreter_->Invoke();
    }
    std::array<float, CLASS_NUM> dists;
    for(int k=0; k<cowRefs_.size(); k++)
        dists[k] = boost::math::tools::l2_distance(
                 cm::model::cvtTensor(c_output_tensor_),cowRefs_[k].feat);
    return std::min_element(dists.begin(),dists.end()) - dists.begin();
}

void CowMonitor::resetFence(){
    for(auto &f: fences_){
        f.cow_id = -1;
        f.cow_box = cv::Rect(-1,-1,-1,-1);
    }
}

auto CowMonitor::mqtt_pub(std::string MSG) -> bool{
    try {
        // Connect to the client
        cli_->connect(connOpts_);
        auto msg = mqtt::make_message("sensors", MSG);
        msg->set_qos(1);
        cli_->publish(msg);
        cli_->disconnect();
        return true;
    }
    catch (const mqtt::exception& exc) {
        std::cerr << "Error: " << exc.what() << " ["
            << exc.get_reason_code() << "]" << std::endl;
        return false;
    }
    return true;
}

/*
 * Start streaming
 * @param
 *     [in] width: capture image width
 *     [in] height: capture image height
 * @return
 *     status of Stream function
 *  */
auto CowMonitor::Stream(int width, int height) -> bool{
    raspicam::RaspiCam_Cv Camera;
    cv::Mat frame;
    Camera.set(cv::CAP_PROP_FORMAT, CV_8UC3);
    Camera.set(cv::CAP_PROP_FRAME_WIDTH, width);
    Camera.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    vW_ = width; vH_ = height;
    std::cout << "====================================\n";
    cout << "Opening Camera...\n";
    if(!Camera.open()){
        cerr << "Error opening the camera\n";
        return -1;
    }
    else std::cout << "Open camera successfully.\n";
    std::cout << "====================================\n";
    std::cout << "Start Streaming...\n";
    for(;;){
        /* setup current time, foldername, .dat file stream */
        std::time_t now = std::time(nullptr);
        char ymd[12], hms[12];
        strftime(ymd, sizeof(ymd), "%Y_%m_%d", std::localtime(&now));
        strftime(hms, sizeof(hms), "%H_%M_%S", std::localtime(&now));
        std::string YMD(ymd), HMS(hms);
        std::string img_dir = "/home/data/img/"+YMD+"/";
        std::filesystem::create_directories(img_dir);
        std::ofstream datFile("/home/data/"+YMD+".dat", std::ios::app);
        if(!datFile.is_open()) cerr << "open .dat failed\n";
        std::cout << '\r' << YMD << '-' << HMS << std::flush;

        /* capture image */
        Camera.grab();
        Camera.retrieve(frame);
        cv::flip(frame, frame, -1);
        cv::Mat image=frame.clone();

        /* pre-declare result variable */
        resetFence();
        vector<cv::Rect> result_box;

        /* if detection have any result */
        if(detection(image, result_box)){
            int total = 0;
            for(cv::Rect box: result_box){
                for(Fence &f: fences_){
                    if((box & f.bbox).area()/box.area() > 0.5){
                        f.cow_id = cowRefs_[classification(image(box))].id;
                        f.cow_box = box;
                        total ++;
                    }
                }
            }

            if(total != 0){
                /* log output */
                std::cout << '\t' << total << " ";
                for(Fence &f: fences_)
                    std::cout << f.f_id << ":" << f.cow_id << " ";
                std::cout << '\n';

                /* make mqtt messege */
                std::stringstream MSG;

                MSG << "NTU_FEED_INDV,node=" << node_ << " ";
                for(auto f:fences_){
                    MSG << "f" << f.f_id << "=" << f.cow_id << ","
                        << "b" << f.f_id << "=" << "\""
                        << f.cow_box.x << ","
                        << f.cow_box.y << ","
                        << f.cow_box.width << ","
                        << f.cow_box.height << "\",";
                    if(f.cow_id != -1)
                        MSG << f.cow_id << "=1,";
                }
                MSG << "total=" << total << " " << now << "000000000\n";

                 /* mqtt publish */
                mqtt_pub(MSG.str());
                datFile << MSG.str();
                datFile.flush();
                /* save image */
                cv::imwrite(img_dir+YMD+"-"+HMS+".jpg", frame);
            }
        }
        else{
            std::cout << '\t' << 0 << " ";
            for(Fence &f: fences_)
                std::cout << f.f_id << ":" << f.cow_id << " ";
        }
    }
    Camera.release();
    return true;
}

auto CowMonitor::RunImage(std::string fileName) -> void{
    cv::Mat img = cv::imread(fileName);
    vW_ = img.cols; vH_ = img.rows;
    vector<cv::Rect> result_box;
    std::cout << "start detect\n";
    if(detection(img, result_box)){
        for(cv::Rect box:result_box)
            cv::rectangle(img, box, cv::Scalar(0, 255, 0), 3);
        for(Fence f: fences_){
            cv::rectangle(img, f.bbox, cv::Scalar(255, 0, 0), 3);
        }
        cv::imwrite("./result.jpg", img);
        std::cout << "start classify\n";
        for(cv::Rect box: result_box){
            for(Fence &f: fences_){
                if((box & f.bbox).area()/box.area() > 0.5){
                    f.cow_id = cowRefs_[classification(img(box))].id;
                }
            }
        }
        for(Fence &f: fences_){
            std::cout << "f_id" << f.f_id << ",";
            std::cout << "cow_id" << f.cow_id << '\n';
        }
    }
}

/*=================== Other namespace definition ===================*/

auto cm::model::readTSV(std::string file) -> std::vector< std::vector<float> >{
    std::vector< std::vector<float> > vecs;
    std::ifstream inFile(file);
    std::string line;

    while(getline(inFile, line)){
        std::stringstream ss(line);
        std::vector<float> vec;
        std::string tmp;
        while(getline(ss, tmp, '\t')) {
            vec.emplace_back(std::stof(tmp));
        }
        vecs.emplace_back(vec);
    }
    return vecs;
}

auto cm::model::readCSV(std::string file) -> std::vector< std::vector<int> >{
    std::vector< std::vector<int> > vecs;
    std::ifstream csvFile(file);
    std::string line;
    while(getline(csvFile, line)){
        std::stringstream ss(line);
        std::vector<int> vec;
        std::string tmp;
        while(getline(ss, tmp, ',')){
            vec.emplace_back(std::stoi(tmp));
        }
        vecs.emplace_back(vec);
    }
    return vecs;
}

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
