#ifndef __COW_MONITOR_H__
#define __COW_MONITOR_H__

#include "yolov4-tiny/yolov4-tiny.h"
#include "mobilenetv2/mobilenetv2.h"
#include <iostream>
#include <array>
#include <deque>


#include "mqtt/client.h"
#define CLASS_NUM 19
#define N_FENCE 3    // max number of fence

using namespace std;


class CowMonitor{
    struct Fence{
        /* fence info */
        int f_id;
        cv::Rect bbox;
        /* cow info */
        int cow_id = -1;
        cv::Rect cow_box;
        float min_d = 0;
    };
    struct CowRef{
        int id;
        std::vector<float> feat;
    };
    public:
        CowMonitor(){};
        bool Init(std::map<std::string, std::string> conf_map);
        bool Stream(int width=1280, int height=960);
        void RunImage(std::string directory);


    private:
        int cal_l2(std::vector<float> input, float &min_d);
        bool initCowRefs(std::string ref_path, std::string dict_path);
        bool initFenceCfg(std::string fence_path);
        void resetFence();
        bool recognize_pipeline(cv::Mat image, int &total);
        std::string create_msg(std::string db_table, time_t now, int total);
        bool mqtt_pub(std::string MSG);

        int vW_, vH_;
        std::unique_ptr<Yolov4Tiny > detect_model_;
        std::unique_ptr<MobileNetv2> classify_model_;

        std::array<Fence, N_FENCE> fences_;
        std::array<CowRef, CLASS_NUM> cowRefs_;
        std::unique_ptr<mqtt::client> cli_;
        mqtt::connect_options connOpts_;
        std::string node_  = "";
        std::string ip_    = "";
        std::string user_  = "";
        std::string pwd_   = "";
};

template<typename T>
auto readTSV(std::string file) -> std::vector< std::vector<T> >;
auto readTSV(std::string file) -> std::vector< std::vector<float> >;

template<typename T>
auto readCSV(std::string file) -> std::vector< std::vector<T> >;
auto readCSV(std::string file) -> std::vector< std::vector<int> >;

#endif /* __COW_MONITOR_H__ */
