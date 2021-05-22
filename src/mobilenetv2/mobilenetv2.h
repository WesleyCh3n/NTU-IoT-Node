#ifndef __MOBILENETV2_H__
#define __MOBILENETV2_H__

#include "network.h"

class MobileNetv2{
    public:
        MobileNetv2(std::string model_path);
        std::vector<float> invoke(cv::Mat inputImg);

    private:
        std::unique_ptr<tflite::FlatBufferModel> model_;
        std::unique_ptr<tflite::Interpreter> interpreter_;
        TfLiteTensor* inp_tensor_ = nullptr;
        TfLiteTensor* out_tensor_= nullptr;
};


#endif
