#ifndef __YOLOV4TINY_H__
#define __YOLOV4TINY_H__

#include "network/network.h"

class Yolov4Tiny{
    public:
        Yolov4Tiny(std::string model_path, float conf_thres, float nms_thres,
                   int img_w, int img_h);
        bool invoke(cv::Mat inputImg, std::vector<cv::Rect> &result_box);
    private:
        bool postprocess(std::vector<float> &box, std::vector<float> &score,
                         std::vector<cv::Rect> &result);

        const int W_;
        const int H_;
        const float conf_thres_;
        const float nms_thres_;
        std::unique_ptr<tflite::FlatBufferModel> model_;
        std::unique_ptr<tflite::Interpreter> interpreter_;
        TfLiteTensor* inp_tensor_ = nullptr;
        TfLiteTensor* out_tensor_box_   = nullptr;
        TfLiteTensor* out_tensor_score_ = nullptr;
};


#endif
