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

typedef cv::Point3_<float> Pixel;

template<typename T>
auto cvtTensor(TfLiteTensor* tensor) -> vector<T>;

class CowMonitor{
    public:
        CowMonitor(){
        };

        auto Init(const std::string& dmodel_path) -> bool;
        auto matPreprocess(cv::Mat &src, uint width, uint height) -> cv::Mat;

        auto yoloResult(std::vector<float> &box,
                        std::vector<float> &score,
                        float thres,
                        std::vector<cv::Rect> &result) -> bool;

        auto Stream(int width=1280, int height=960) -> bool;
        auto RunImage(std::string fileName);

    private:
        uint height() const;
        uint width() const;
        uint input_channels() const;
        std::unique_ptr<tflite::FlatBufferModel> model;
        std::unique_ptr<tflite::Interpreter> interpreter;
        TfLiteTensor* input_tensor = nullptr;
        TfLiteTensor* output_box = nullptr;
        TfLiteTensor* output_score = nullptr;
        int vW = 1280;
        int vH = 960;
};
#endif /* COW_MONITOR_H */
