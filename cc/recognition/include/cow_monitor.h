#ifndef COW_MONITOR_H
#define COW_MONITOR_H

#include <iostream>
#include <vector>

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"

#include "opencv2/opencv.hpp"

using namespace std;


namespace cm{

    typedef cv::Point3_<float> Pixel;

    const int DETECT    = 0;
    const int CLASSIFY  = 1;
    const int RECOGNIZE = 2;

    class CowMonitor{
        struct InputDim{
            uint height;
            uint width;
            uint channel;
        };
        public:
            CowMonitor(){};

            auto Init(std::string model_path[], int MODE) -> bool;
            auto initdModel(std::string model_path) -> bool;
            auto initcModel(std::string model_path) -> bool;
            auto matPreprocess(cv::Mat &src, uint width, uint height,
                               void (*norm)(Pixel&)) -> cv::Mat;
            auto yoloResult(std::vector<float> &box, std::vector<float> &score,
                            float thres, std::vector<cv::Rect> &result) -> bool;
            auto detection(cv::Mat inputImg,
                           std::vector<cv::Rect> &result_box) -> bool;
            auto classification(cv::Mat inputImg,
                                std::vector<cv::Rect> result_box) -> bool;
            auto Stream(int width=1280, int height=960) -> bool;
            auto RunImage(std::string fileName) -> void;

        private:
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
            int vW_, vH_;
    };

    namespace model{
        template<typename T>
        auto cvtTensor(TfLiteTensor* tensor) -> std::vector<T>;
        auto cvtTensor(TfLiteTensor* tensor) -> std::vector<float>;

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
