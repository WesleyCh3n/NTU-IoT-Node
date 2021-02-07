/* tflite model:
 *     - input shape: (1,224,224,3)
 *     - output shape: (1,128)
 * In C++, cv::Mat should be flatten as input. If input has 3 channel,
 * then it should be RGBRGBRGB... */

#include <iostream>
#include <iomanip>

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"

#include "opencv2/opencv.hpp"

#define LOG(x) std::cout << x << " ";
using namespace std;
typedef cv::Point3_<float> Pixel;


void normalize(Pixel &pixel){
    pixel.x = ((pixel.x / 255.0)-0.5)*2.0;
    pixel.y = ((pixel.y / 255.0)-0.5)*2.0;
    pixel.z = ((pixel.z / 255.0)-0.5)*2.0;
}

int main(){
    // create model
	std::unique_ptr<tflite::FlatBufferModel> model =
        tflite::FlatBufferModel::BuildFromFile("../../model/mobilenetv2-128.tflite");
	tflite::ops::builtin::BuiltinOpResolver resolver;
	std::unique_ptr<tflite::Interpreter> interpreter;
	tflite::InterpreterBuilder(*model.get(), resolver)(&interpreter);
	interpreter->AllocateTensors();

    // get input & output layer
	float* inputLayer = interpreter->typed_input_tensor<float>(0);
	float* outputLayer = interpreter->typed_output_tensor<float>(0);

    TfLiteTensor* input_tensor = interpreter->tensor(interpreter->inputs()[0]);
    TfLiteTensor* output_tensor = interpreter->tensor(interpreter->outputs()[0]);

    // ======================
    // TfLiteTensor struct
    //     -> dims -> size
    //             -> data[]
    //     -> data
    // ======================

    for(int i=0; i<input_tensor->dims->size; i++){
        LOG(input_tensor->dims->data[i]);
    }
    std::cout << '\n';
    // Output: 1 224 224 3

    int height = input_tensor->dims->data[1];
    int width = input_tensor->dims->data[2];
    int channel = input_tensor->dims->data[3];

    // read image file
    cv::Mat img = cv::imread("01.jpg");

    // convert to float; BGR -> RGB
    cv::Mat inputImg;
    img.convertTo(inputImg, CV_32FC3);
    cv::cvtColor(inputImg, inputImg, cv::COLOR_BGR2RGB);

    // normalize to -1 & 1
    Pixel* pixel = inputImg.ptr<Pixel>(0,0);
    const Pixel* endPixel = pixel + inputImg.cols * inputImg.rows;
    for (; pixel != endPixel; pixel++)
        normalize(*pixel);

    // resize image as model input
    cv::resize(inputImg, inputImg, cv::Size(width, height));

    // flatten rgb image to input layer.
    memcpy(input_tensor->data.f, inputImg.ptr<float>(0),
           height * height * channel * sizeof(float));

    // compute model instance
    interpreter->Invoke();

    for(int i=0; i<output_tensor->dims->size; i++){
        LOG(output_tensor->dims->data[i]);
    }
    std::cout << '\n';
    // Output: 1 128

    printf("[");
    for(int i=1; i<=output_tensor->dims->data[1]; i++){
        cout << setw(11) << fixed << setprecision(8)
             << output_tensor->data.f[i-1] << " ";
        if(i%6==0) printf("\n ");
    }
    printf("]\n");
    // Output:
    // [ 0.11753587 -0.03103371 -0.12719961  0.04908999 -0.06446263 -0.00487081
    //   0.12962814 -0.02103156 -0.03182427 -0.06548748 -0.06392512 -0.09980485
    //  -0.13455814 -0.15724786 -0.07408750  0.01878389  0.04111398  0.02339300
    //   0.09919989 -0.06832353 -0.07597472  0.08902493 -0.08702157 -0.13256179
    //  -0.12643440 -0.04555996 -0.14587280  0.05495473 -0.09340401 -0.00094481
    //  -0.01205552 -0.10978770  0.06796338 -0.12493090 -0.18242088 -0.01653432
    //   0.04594451 -0.06405982 -0.08853709  0.16529705  0.20050898 -0.00548758
    //  -0.08511856  0.05009530 -0.12339530 -0.00691709 -0.03817068  0.02417666
    //   0.04600707 -0.02206794 -0.03188995  0.02405100  0.12199991  0.04503687
    //   0.07581869  0.07498807 -0.18143463  0.10554960 -0.05890078 -0.02717691
    //   0.06061919  0.11713787  0.06222145  0.01504421  0.06767056 -0.09617412
    //  -0.09902407 -0.07601757  0.09401433  0.11793025  0.05398557 -0.04995451
    //  -0.06455081 -0.06965023  0.10391773 -0.11351185 -0.10195548  0.08580037
    //   0.01418317 -0.02287764 -0.12372399  0.02075163 -0.06046351  0.07479008
    //   0.05404664  0.13760927  0.10700486  0.07360139  0.13396731 -0.11350382
    //  -0.10615805 -0.13753231 -0.10298547 -0.11060565 -0.14629062 -0.07289285
    //  -0.00845566 -0.07271316  0.07144255  0.13439240  0.08693192  0.07452447
    //  -0.08722545 -0.13260794  0.04353381 -0.04652282 -0.04917885 -0.15425837
    //   0.05380632  0.02613331  0.02936596  0.04128572 -0.04925763  0.10147007
    //   0.06830482 -0.19268657 -0.02124127  0.08511845  0.06944570 -0.12235070
    //  -0.08086106  0.02228669  0.03113141 -0.08988636  0.00600719  0.07516700
    //   0.05946938 -0.00403670 ]
}
