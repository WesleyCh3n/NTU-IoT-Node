#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <vector>
#include "opencv2/opencv.hpp"

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"

typedef cv::Point3_<float> Pixel;

cv::Mat matPreprocess(cv::Mat &src, uint width, uint height, 
                      void (*norm)(Pixel&));
std::vector<float> cvtTensor(TfLiteTensor* tensor);
void yolov4_norm(Pixel &pixel);
void mobilenetv2_norm(Pixel &pixel);

#endif
