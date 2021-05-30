#include "src/cow_monitor.h"
#include "src/timer.h"

#include <set>
#include <algorithm>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdexcept>


#include "boost/format.hpp"
#include "boost/math/tools/norms.hpp"
#include "raspicam/raspicam_cv.h"
#include "mqtt/client.h"
using namespace std;
namespace fs = std::filesystem;

const char *VERSION = NTU_IOT_NODE_VERSION;

/*=================== CowMonitor Class definition ===================*/

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
bool CowMonitor::Init(std::map<std::string, std::string> conf_map){
    node_ = conf_map.at("node");

    if(!initCowRefs(conf_map.at("ref"), conf_map.at("ref_dict")))
        return false;
    if(!initFenceCfg(conf_map.at("fence")))
        return false;

    cli_ = std::make_unique<mqtt::client>("tcp://" + conf_map.at("mqtt_ip"), "node" + node_);
    connOpts_.set_keep_alive_interval(20);
    connOpts_.set_clean_session(true);
    connOpts_.set_user_name(conf_map.at("user"));
    connOpts_.set_password(conf_map.at("pwd"));

    std::cout << std::left << "====================================\n"
              << std::setw(16) << "Version" << ": " << VERSION << '\n'
              << std::setw(16) << "Node" << ": " << node_ << '\n'
              << std::setw(16) << "Mqtt" << ": " << "tcp://"+conf_map.at("mqtt_ip") << '\n'
              << std::setw(16) << "Mqtt user" << ": " << conf_map.at("user") << '\n'
              << std::setw(16) << "Fence" << ": " << conf_map.at("fence") << '\n'
              << std::setw(16) << "Ref" << ": " << conf_map.at("ref") << '\n'
              << std::setw(16) << "Ref dict" << ": " << conf_map.at("ref_dict") << '\n'
              << "====================================\n";
    detect_model_ = std::make_unique<Yolov4Tiny>(conf_map.at("detect"),
                                                 0.6, 0.4, 1280, 960);
    classify_model_ = std::make_unique<MobileNetv2 >(conf_map.at("classify"));
    std::cout << "====================================\n";
    return true;
}

bool CowMonitor::initCowRefs(std::string ref_path, std::string dict_path) {
    if(!fs::exists(ref_path) && !fs::exists(dict_path))
        return false;
    std::vector< std::vector<float> > feats = readTSV(ref_path);
    std::ifstream inFile(dict_path);
    std::string id;
    int i = 0;
    while(getline(inFile, id)){
        cowRefs_[i].id = std::stoi(id);
        cowRefs_[i].feat = feats[i];
        i++;
    }
    return true;
}

bool CowMonitor::initFenceCfg(std::string fence_path){
    if(!fs::exists(fence_path))
        return false;
    std::vector< std::vector<int> > bboxes = readCSV(fence_path);
    for(int i=0; i<bboxes.size(); i++){
        fences_[i].f_id = i;
        int x = bboxes[i][0];
        int y = bboxes[i][1];
        int w = bboxes[i][2] - x;
        int h = bboxes[i][3] - y;
        fences_[i].bbox = cv::Rect(x, y, w, h);
    }
    return true;
}


int CowMonitor::cal_l2(std::vector<float> input, float &min_d){
    std::array<float, CLASS_NUM> dists;
    for(int k=0; k<cowRefs_.size(); k++)
        dists[k] = boost::math::tools::l2_distance(input,cowRefs_[k].feat);
    int index = std::min_element(dists.begin(),dists.end()) - dists.begin();
    min_d = dists[index];
    return index;
}


void CowMonitor::resetFence(){
    for(auto &f: fences_){
        f.cow_id = -1;
        f.cow_box = cv::Rect(-1,-1,-1,-1);
        f.min_d = 0;
    }
}


bool CowMonitor::mqtt_pub(std::string MSG){
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


bool CowMonitor::recognize_pipeline(cv::Mat image, int &total){
    /* pre-declare result variable */
    vector<cv::Rect> result_box;
    total = 0;
    if (detect_model_->invoke(image, result_box)) {
        /* iterate all fence */
        for(Fence &f: fences_){
            /* test which result box in this fence */
            for(cv::Rect box: result_box){
                if((box & f.bbox).area()/box.area() > 0.5){
                    int index = cal_l2(classify_model_->invoke(image(box)), f.min_d);
                    f.cow_id = cowRefs_[index].id;
                    f.cow_box = box;
                    total ++;
                }
            }
        }
        if (total > 0) return true;
    }
    return false;
}

/*
 * Start streaming
 * @param
 *     [in] width: capture image width
 *     [in] height: capture image height
 * @return
 *     status of Stream function
 *  */
bool CowMonitor::Stream(int width, int height){
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
    else std::cout << "Open camera successfully.\n";
    std::cout << "====================================\n";
    std::cout << "Start Streaming...\n";
    for(;;){
        /* capture image */
        Camera.grab();
        Camera.retrieve(frame);
        cv::flip(frame, frame, -1);
        cv::Mat image=frame.clone();

        /* setup current time */
        std::time_t now = std::time(nullptr);
        char ymd[12], hms[12];
        strftime(ymd, sizeof(ymd), "%Y_%m_%d", std::localtime(&now));
        strftime(hms, sizeof(hms), "%H_%M_%S", std::localtime(&now));
        std::string YMD(ymd), HMS(hms);
        /* setup save image filder */
        std::string img_dir = "./data/img/"+YMD+"/";
        fs::create_directories(img_dir);
        /* setup .dat */
        std::ofstream datFile("/home/data/"+YMD+".dat", std::ios::app);

        resetFence();
        int total;
        if(recognize_pipeline(image, total)){
            std::string msg = create_msg("NTU_FEED_INDV", now, total);
            mqtt_pub(msg);
            datFile << msg;
            datFile.flush();
            /* save image */
            cv::imwrite(img_dir+YMD+"-"+HMS+".jpg", frame);
        }
        std::cout << YMD << '-' << HMS << '\t' <<  total << " ";
        for(auto &f: fences_)
            std::cout << f.f_id << ":" << f.cow_id << " ";
        std::cout << '\n';
    }
    Camera.release();
    return true;
}

void CowMonitor::RunImage(std::string directory){
    set<fs::path> sorted_by_name;
    for (const auto & entry : fs::directory_iterator(directory))
        sorted_by_name.insert(entry.path());

    for (auto &filename : sorted_by_name){
        cout << filename.c_str() << endl;
        cv::Mat frame = cv::imread(filename.c_str());
        cv::Mat image=frame.clone();
        /* setup current time */
        std::time_t now = std::time(nullptr);
        char ymd[12], hms[12];
        strftime(ymd, sizeof(ymd), "%Y_%m_%d", std::localtime(&now));
        strftime(hms, sizeof(hms), "%H_%M_%S", std::localtime(&now));
        std::string YMD(ymd), HMS(hms);
        /* setup save image filder */
        std::string img_dir = "/home/data/img/"+YMD+"/";
        fs::create_directories(img_dir);
        /* setup .dat */
        std::ofstream datFile("/home/data/"+YMD+".dat", std::ios::app);

        resetFence();
        int total;
        if(recognize_pipeline(image, total)){
            /* create MQTT msg purpose */
            std::string msg = create_msg("test_image", now, total);
             /* mqtt publish */
            mqtt_pub(msg);
            datFile << msg;
            datFile.flush();
        }
        std::cout << YMD << '-' << HMS << '\t' <<  total << " ";
        for(auto &f: fences_)
            std::cout << f.f_id << ":" << f.cow_id << " ";
        std::cout << '\n';
    }
}

std::string CowMonitor::create_msg(std::string db_table, time_t now, int total){
    std::stringstream MSG;
    MSG << db_table <<",node=" << node_ << " ";
    for(auto f: fences_){
        MSG << "f" << f.f_id << "=" << f.cow_id << ","
            << "f_d" << f.f_id << "=" << f.min_d << ","
            << "b" << f.f_id << "=" << "\""
            << f.cow_box.x << "," << f.cow_box.y << ","
            << f.cow_box.width << "," << f.cow_box.height << "\",";
        if(f.cow_id != -1) MSG << f.cow_id << "=1,";
    }
    MSG << "total=" << total << " " << now << "000000000\n";
    return MSG.str();
}

/*=================== Other namespace definition ===================*/

auto readTSV(std::string file) -> std::vector< std::vector<float> >{
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

auto readCSV(std::string file) -> std::vector< std::vector<int> >{
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
