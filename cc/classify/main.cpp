/* tflite model:
 *     - input shape: (224,224,3)
 *     - output shape: (128)
 * In C++, cv::Mat should be flatten as input. If input has 3 channel,
 * then it should be RGBRGBRGB... */

#include <iostream>
#include <iomanip>

#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
// #include "tensorflow/lite/optional_debug_tools.h"

#include "opencv2/opencv.hpp"
using namespace std;
typedef cv::Point3_<float> Pixel;

const uint WIDTH = 224;
const uint HEIGHT = 224;
const uint CHANNEL = 3;
const uint OUTDIM = 128;

void normalize(Pixel &pixel){
    pixel.x = ((pixel.x / 255.0)-0.5)*2.0;
    pixel.y = ((pixel.y / 255.0)-0.5)*2.0;
    pixel.z = ((pixel.z / 255.0)-0.5)*2.0;
}

int main(){
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
    cv::resize(inputImg, inputImg, cv::Size(WIDTH, HEIGHT));

    // create model
	std::unique_ptr<tflite::FlatBufferModel> model =
        tflite::FlatBufferModel::BuildFromFile("model.tflite");
	tflite::ops::builtin::BuiltinOpResolver resolver;
	std::unique_ptr<tflite::Interpreter> interpreter;
	tflite::InterpreterBuilder(*model.get(), resolver)(&interpreter);
	interpreter->AllocateTensors();

    // get input & output layer
	float* inputLayer = interpreter->typed_input_tensor<float>(0);
	float* outputLayer = interpreter->typed_output_tensor<float>(0);

    // flatten rgb image to input layer.
    float* inputImg_ptr = inputImg.ptr<float>(0);
    memcpy(inputLayer, inputImg.ptr<float>(0),
           WIDTH * HEIGHT * CHANNEL * sizeof(float));

    // compute model instance
	interpreter->Invoke();

    // print final result
    printf("[");
    for(int i=1; i<=OUTDIM; i++){
        cout << setw(11) << fixed << setprecision(8)
             << outputLayer[i-1] << " ";
        if(i%6==0) printf("\n ");
    }
    printf("]\n");
}
