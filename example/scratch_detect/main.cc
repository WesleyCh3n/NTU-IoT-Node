#include <iostream>

#include <chrono>
#include <iomanip>
#include <vector>

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"

#include "opencv2/opencv.hpp"
using namespace std;
#define LOG(x) std::cout << x << std::endl;
typedef cv::Point3_<float> Pixel;


void normalize(Pixel &pixel){
    pixel.x = (pixel.x / 255.0);
    pixel.y = (pixel.y / 255.0);
    pixel.z = (pixel.z / 255.0);
}

auto matPreprocess(cv::Mat src, uint width, uint height) -> cv::Mat{
    // convert to float; BGR -> RGB
    cv::Mat dst;
    src.convertTo(dst, CV_32FC3);
    cv::cvtColor(dst, dst, cv::COLOR_BGR2RGB);

    // normalize to -1 & 1
    Pixel* pixel = dst.ptr<Pixel>(0,0);
    const Pixel* endPixel = pixel + dst.cols * dst.rows;
    for (; pixel != endPixel; pixel++)
        normalize(*pixel);

    // resize image as model input
    cv::resize(dst, dst, cv::Size(width, height));
    return dst;
}

template<typename T>
auto cvtTensor(TfLiteTensor* tensor) -> vector<T>;

auto cvtTensor(TfLiteTensor* tensor) -> vector<float>{
    int nelem = 1;
    for(int i=0; i<tensor->dims->size; ++i){
        nelem *= tensor->dims->data[i];
        LOG(tensor->dims->data[i]);
    }
    vector<float> data(tensor->data.f, tensor->data.f+nelem);
    return data;
}

int main(){
    // create model
    std::unique_ptr<tflite::FlatBufferModel> model =
        tflite::FlatBufferModel::BuildFromFile("./yolov4-tiny-int8.tflite");
    tflite::ops::builtin::BuiltinOpResolver resolver;
    std::unique_ptr<tflite::Interpreter> interpreter;
    tflite::InterpreterBuilder(*model.get(), resolver)(&interpreter);
    interpreter->AllocateTensors();

    // get input & output layer
    TfLiteTensor* input_tensor = interpreter->tensor(interpreter->inputs()[0]);
    TfLiteTensor* output_box = interpreter->tensor(interpreter->outputs()[0]);
    TfLiteTensor* output_score = interpreter->tensor(interpreter->outputs()[1]);

    const uint HEIGHT = input_tensor->dims->data[1];
    const uint WIDTH = input_tensor->dims->data[2];
    const uint CHANNEL = input_tensor->dims->data[3];

    // read image file
    cv::Mat img = cv::imread("NODE29.jpg");
    cv::Mat inputImg = matPreprocess(img, WIDTH, HEIGHT);

    double totalt = 0;
    for(int i=0; i<10; i++){
        // flatten rgb image to input layer.
        float* inputImg_ptr = inputImg.ptr<float>(0);
        memcpy(input_tensor->data.f, inputImg.ptr<float>(0),
               WIDTH * HEIGHT * CHANNEL * sizeof(float));

        // compute model instance
        std::chrono::time_point<std::chrono::system_clock> start, end;
        std::chrono::duration<double> elapsed_seconds;
        start = std::chrono::system_clock::now();
        interpreter->Invoke();
        end = std::chrono::system_clock::now();
        elapsed_seconds = end - start;
        printf("s: %.10f\n" ,elapsed_seconds.count());
        totalt += elapsed_seconds.count();
    }
    LOG("ave");
    LOG(totalt/10);

    vector<float> box_vec = cvtTensor(output_box);
    vector<float> score_vec = cvtTensor(output_score);


    vector<size_t> result_id;
    auto it = std::find_if(std::begin(score_vec), std::end(score_vec),
                           [](float i){return i > 0.6;});
    while (it != std::end(score_vec)) {
        result_id.emplace_back(std::distance(std::begin(score_vec), it));
        it = std::find_if(std::next(it), std::end(score_vec),
                          [](float i){return i > 0.6;});
    }

    vector<cv::Rect> rects;
    vector<float> scores;
    for(size_t tmp:result_id){
        const int cx = box_vec[4*tmp];
        const int cy = box_vec[4*tmp+1];
        const int w = box_vec[4*tmp+2];
        const int h = box_vec[4*tmp+3];
        const int xmin = ((cx-(w/2.f))/WIDTH) * img.cols;
        const int ymin = ((cy-(h/2.f))/HEIGHT) * img.rows;
        const int xmax = ((cx+(w/2.f))/WIDTH) * img.cols;
        const int ymax = ((cy+(h/2.f))/HEIGHT) * img.rows;
        rects.emplace_back(cv::Rect(xmin, ymin, xmax-xmin, ymax-ymin));
        scores.emplace_back(score_vec[tmp]);
    }

    vector<int> ids;
    cv::dnn::NMSBoxes(rects, scores, 0.6, 0.4, ids);
    for(int tmp: ids){
        std::cout << rects[tmp];
        cv::rectangle(img, rects[tmp], cv::Scalar(0, 255, 0), 3);
    }
    cv::imwrite("./result.jpg", img);
    return 0;
}
