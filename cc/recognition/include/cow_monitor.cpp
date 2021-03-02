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

bool CowMonitor::Init(std::string node, std::string model_path[], std::string ref, int mode){
    node_ = node;
    std::cout << std::left << "====================================\n"
              << std::setw(16) << "Version" << ": " << VERSION << '\n'
              << std::setw(16) << "Node" << ": " << node_ << '\n'
              << std::setw(16) << "Mode" << ": " << mode << '\n'
              << std::setw(16) << "Mqtt" << ": " << ip_ << '\n'
              << std::setw(16) << "Mqtt user" << ": " << user_ << '\n';
    if(!ref.empty()) refs_ = cm::model::readTSV(ref);
    switch(mode){
        case cm::DETECT:{
            bool d_status = initdModel(model_path[0]);
            return d_status;
            break;
        }
        case cm::CLASSIFY:{
            bool s_status = initcModel(model_path[1]);
            return s_status;
            break;
        }
        case cm::RECOGNIZE:{
            bool d_status = initdModel(model_path[0]);
            bool s_status = initcModel(model_path[1]);
            return (d_status && s_status);
            break;
        }
    }
    return false;
}


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

/**
 * Find valid bbox by confidence and non-maximum suppression
 *
 * @param[in] box: result of yolo bbox
 * @param[in] score: result of yolo score
 * @param[out] result: valid bbox with cv::Rect(xmin,xmax,w,h) as vector
 */
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

auto CowMonitor::classification(cv::Mat inputImg,
                                std::vector<cv::Rect> detect_box,
                                std::array<int, MAX_NUM_PF> &result) -> void{
    /* TODO: Use stack method with fix output size(128) & result(6)
     * std::array<std::array<float, 128>, 6> results; */
#ifdef BENCHMARK
    Timer timer;
#endif
    int i=0;
    for(cv::Rect roi: detect_box){
        if(i > MAX_NUM_PF-1) break;
        cv::Mat cropImg = inputImg(roi);
        cropImg = matPreprocess(cropImg,
                c_input_dim_.w, c_input_dim_.h,
                cm::model::mobilenetv2::norm);
        memcpy(c_input_tensor_->data.f, cropImg.ptr<float>(0),
                c_input_dim_.h * c_input_dim_.h *
                c_input_dim_.c * sizeof(float));
        {
#ifdef BENCHMARK
            Timer timer;
#endif
            c_interpreter_->Invoke();
        }
        std::array<float, CLASS_NUM> tmp;
        for(int k=0; k<refs_.size(); k++)
            tmp[k] = boost::math::tools::l2_distance(cm::model::cvtTensor(c_output_tensor_),refs_[k]);
        result[i] = std::min_element(tmp.begin(),tmp.end()) - tmp.begin();
        i++;
    }
}

auto CowMonitor::mqtt_pub(std::time_t &now, std::vector<cv::Rect> result_box,
                          std::array<int, MAX_NUM_PF> &result_ids,
                          std::string &msgOut) -> bool{
    cv::Rect nBox(-1,-1,-1,-1);
    std::stringstream MSG;
    int total = result_box.size();
    if(total < MAX_NUM_PF + 1)
        std::fill_n(std::back_inserter(result_box), MAX_NUM_PF-total, nBox);
    MSG << "NTU_FEED,node=" << node_ << " total=" << total << ",";
    for(size_t i=0; i<MAX_NUM_PF; i++){
        MSG << "id" << i << "=" << result_ids[i] << ","
            << "box" << i << "=" << "\"" << result_box[i].x << ","
            << result_box[i].y << "," << result_box[i].width << ","
            << result_box[i].height << "\"";
        if(i != MAX_NUM_PF-1) MSG << ",";
    }
    MSG << " " << now << "000000000";
    msgOut = MSG.str();
    try {
        // Connect to the client
        cli_->connect(connOpts_);
        auto msg = mqtt::make_message("sensors", MSG.str());
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
}

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

        /* pre-declare result variable */
        vector<cv::Rect> result_box;
        std::array<int,MAX_NUM_PF> result_ids;
        result_ids.fill(-1);

        /* if detect result is true -> classify */
        if(detection(frame, result_box)){
            classification(frame, result_box, result_ids);

            /* log output */
            std::cout << '\t' << result_box.size() << " ";
            for(auto &id:result_ids) std::cout << id << " ";
            std::cout << '\n';

             /* mqtt publish */
            std::string msg;
            if(mqtt_pub(now, result_box, result_ids, msg)){
                datFile << msg << "\n";
                datFile.flush();
            }

            /* save image */
            cv::imwrite(img_dir+YMD+"-"+HMS+".jpg", frame);
        }
        else{
            std::cout << '\t' << result_box.size();
            for(auto &id:result_ids) std::cout << id << " ";
        }
    }
    Camera.release();
    return true;
}

auto CowMonitor::RunImage(std::string fileName) -> void{
    cv::Mat img = cv::imread(fileName);
    vW_ = img.cols; vH_ = img.rows;
    cv::Rect nValue(-1,-1,-1,-1);
    vector<cv::Rect> result_box;
    if(detection(img, result_box)){
        for(cv::Rect box:result_box)
            cv::rectangle(img, box, cv::Scalar(0, 255, 0), 3);
        cv::imwrite("./result.jpg", img);
        std::array<int,MAX_NUM_PF> result_ids;
        std::fill_n(result_ids.begin(), MAX_NUM_PF, -1);
        classification(img, result_box, result_ids);
        {
            std::ofstream datFile("./detect.csv", std::ios::app);
            Timer timer;
            std::fill_n(std::back_inserter(result_box),
                        MAX_NUM_PF-result_box.size(), nValue);
            for(int &id:result_ids){
                datFile << id << ",";
            }
            std::for_each(result_box.begin(), result_box.end(),
                    [&](cv::Rect &tmp){ datFile << tmp.x << ";"
                                                << tmp.y << ";"
                                                << tmp.width << ";"
                                                << tmp.height << ","; });
            datFile << "\n";
        }
        for(auto id:result_ids)
            std::cout << id << " ";
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
