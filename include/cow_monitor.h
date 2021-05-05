#ifndef COW_MONITOR_H
#define COW_MONITOR_H

#include <iostream>
#include <vector>
#include <array>

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"

#include "opencv2/opencv.hpp"
#include "mqtt/client.h"
#define CLASS_NUM 19
#define N_FENCE 3    // max number of fence

using namespace std;


namespace cm{

    typedef cv::Point3_<float> Pixel;

    const int DETECT    = 0;
    const int CLASSIFY  = 1;
    const int RECOGNIZE = 2;

    class CowMonitor{
        struct InputDim{
            uint h, w, c;
        };
        struct Fence{
            int f_id;
            cv::Rect bbox;
            int cow_id=-1;
            cv::Rect cow_box(-1,-1,-1,-1);
        };
        struct CowRef{
            int id;
            std::vector<float> feat;
        };
        public:
            CowMonitor(){};
            auto InitMqtt(std::string ip, std::string user, std::string pwd) -> void;
            auto Init(std::string node, std::string model_path[],
                      std::string ref_path, std::string dict_path,
                      std::string fence_path, int mode) -> bool;
            void initCowRefs(std::string ref_path, std::string dict_path);
            void initFenceCfg(std::string fence_path);
            auto Stream(int width=1280, int height=960) -> bool;
            auto RunImage(std::string fileName) -> void;


        private:
            auto initdModel(std::string model_path) -> bool;
            auto initcModel(std::string model_path) -> bool;
            auto matPreprocess(cv::Mat &src, uint width, uint height,
                               void (*norm)(Pixel&)) -> cv::Mat;
            auto yoloResult(std::vector<float> &box, std::vector<float> &score,
                            float thres, std::vector<cv::Rect> &result) -> bool;
            auto detection(cv::Mat inputImg,
                           std::vector<cv::Rect> &result_box) -> bool;
            auto classification(cv::Mat inputImg) -> int;
            void resetFence();
            auto mqtt_pub(std::time_t &now, std::vector<cv::Rect> result_box,
                          std::array<int, N_FENCE> &result,
                          std::string &msgOut) -> bool;

            int vW_, vH_;
            std::array<Fence, N_FENCE> fences_;
            std::array<CowRef, CLASS_NUM> cowRefs_;
            InputDim d_input_dim_;
            std::unique_ptr<tflite::FlatBufferModel> d_model_;
            std::unique_ptr<tflite::Interpreter> d_interpreter_;
            TfLiteTensor* d_input_tensor_ = nullptr;
            TfLiteTensor* d_output_box_   = nullptr;
            TfLiteTensor* d_output_score_ = nullptr;
            InputDim c_input_dim_;
            std::unique_ptr<tflite::FlatBufferModel> c_model_;
            std::unique_ptr<tflite::Interpreter> c_interpreter_;
            TfLiteTensor* c_input_tensor_  = nullptr;
            TfLiteTensor* c_output_tensor_ = nullptr;
            /* mqtt variable */
            mqtt::client *cli_ = nullptr;
            mqtt::connect_options connOpts_;
            std::string node_  = "";
            std::string ip_    = "";
            std::string user_  = "";
            std::string pwd_   = "";
    };

    namespace model{
        template<typename T>
        auto cvtTensor(TfLiteTensor* tensor) -> std::vector<T>;
        auto cvtTensor(TfLiteTensor* tensor) -> std::vector<float>;

        template<typename T>
        auto readTSV(std::string file) -> std::vector< std::vector<T> >;
        auto readTSV(std::string file) -> std::vector< std::vector<float> >;

        template<typename T>
        auto readCSV(std::string file) -> std::vector< std::vector<T> >;
        auto readCSV(std::string file) -> std::vector< std::vector<int> >;

        namespace yolov4{
            const float CON_THRES = 0.6;
            const float NMS_THRES = 0.4;
            void norm(Pixel &);
        }
        namespace mobilenetv2{
            void norm(Pixel &);
        }
    }
}
#endif /* COW_MONITOR_H */
