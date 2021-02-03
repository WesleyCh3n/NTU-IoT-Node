#include <iostream>

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <vector>

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"

#include "opencv2/opencv.hpp"
#include "raspicam/raspicam_cv.h"
#include "NumCpp.hpp"
using namespace std;
typedef cv::Point3_<float> Pixel;


void normalize(Pixel &pixel){
    pixel.x = (pixel.x / 255.0);
    pixel.y = (pixel.y / 255.0);
    pixel.z = (pixel.z / 255.0);
}


template<typename T>
auto cvtTensor(TfLiteTensor* tensor) -> vector<T>;

auto cvtTensor(TfLiteTensor* tensor) -> vector<float>{
    int nelem = 1;
    for(int i=0; i<tensor->dims->size; ++i)
        nelem *= tensor->dims->data[i];
    vector<float> data(tensor->data.f, tensor->data.f+nelem);
    return data;
}
class CowMonitor{
    public:
        CowMonitor(){
        };

        auto Init(const std::string& dmodel_path) -> bool{
            printf("Initialize Model\n");
            this->model =
                tflite::FlatBufferModel::BuildFromFile(dmodel_path.c_str());
            tflite::ops::builtin::BuiltinOpResolver resolver;
            tflite::InterpreterBuilder(*this->model.get(), resolver)
                                      (&this->interpreter);
            if (!this->interpreter) {
                cerr << "Failed to create interpreter!";
                return false;
            }
            this->interpreter->AllocateTensors();

            // get input & output layer
            input_tensor = this->interpreter->tensor(interpreter->inputs()[0]);
            output_box = this->interpreter->tensor(interpreter->outputs()[0]);
            output_score = this->interpreter->tensor(interpreter->outputs()[1]);
            return true;
        }

        auto matPreprocess(cv::Mat &src, uint width, uint height) -> cv::Mat{
            // convert to float; BGR -> RGB
            cv::Mat dst;
            src.convertTo(dst, CV_32FC3);
            cv::cvtColor(dst, dst, cv::COLOR_BGR2RGB);

            // normalize
            Pixel* pixel = dst.ptr<Pixel>(0,0);
            const Pixel* endPixel = pixel + dst.cols * dst.rows;
            for (; pixel != endPixel; pixel++)
                normalize(*pixel);

            // resize image as model input
            cv::resize(dst, dst, cv::Size(width, height));
            return dst;
        }

        auto yoloResult(vector<float> &box,
                        vector<float> &score,
                        float thres,
                        vector<cv::Rect> &result) -> bool{

            auto it = std::find_if(std::begin(score), std::end(score),
                                   [&thres](float i){return i > thres;});
            vector<cv::Rect> rects;
            vector<float> scores;
            while (it != std::end(score)) {
                size_t id = std::distance(std::begin(score), it);
                const int cx = box[4*id];
                const int cy = box[4*id+1];
                const int w = box[4*id+2];
                const int h = box[4*id+3];
                const int xmin = ((cx-(w/2.f))/this->width())*this->vW;
                const int ymin = ((cy-(h/2.f))/this->height())*this->vH;
                const int xmax = ((cx+(w/2.f))/this->width())*this->vW;
                const int ymax = ((cy+(h/2.f))/this->height())*this->vH;
                rects.emplace_back(cv::Rect(xmin, ymin, xmax-xmin, ymax-ymin));
                scores.emplace_back(score[id]);
                it = std::find_if(std::next(it), std::end(score),
                                  [&thres](float i){return i > thres;});
            }
            if(rects.empty())
                return false;

            vector<int> ids;
            cv::dnn::NMSBoxes(rects, scores, thres, 0.4, ids);
            if(ids.empty())
                return false;
            for(int tmp: ids){
                result.emplace_back(rects[tmp]);
            }
            return true;
        }

        auto Stream(int width=1280, int height=960) -> bool{
            std::chrono::time_point<std::chrono::system_clock> start, end;
            std::chrono::duration<double> elapsed_seconds;
            raspicam::RaspiCam_Cv Camera;
            cv::Mat frame;
            Camera.set(cv::CAP_PROP_FORMAT, CV_8UC3);
            Camera.set(cv::CAP_PROP_FRAME_WIDTH, width);
            Camera.set(cv::CAP_PROP_FRAME_HEIGHT, height);
            cout << "Opening Camera...\n";
            if(!Camera.open()){
                cerr << "Error opening the camera\n";
                return -1;
            }
            for(;;){
                // start = std::chrono::system_clock::now();
                time_t now = time(0);
                tm *ltm = localtime(&now);
                char ymd[20];
                snprintf(ymd, sizeof(ymd),
                         "%04d_%02d_%02d",
                         1900 + ltm->tm_year,
                         1 + ltm->tm_mon,
                         ltm->tm_mday);
                std::string YMD=ymd;
                char time_f[20];
                snprintf(time_f, sizeof(time_f),
                         "%04d_%02d_%02d-%02d_%02d_%02d",
                         1900 + ltm->tm_year,
                         1 + ltm->tm_mon,
                         ltm->tm_mday,
                         ltm->tm_hour,
                         ltm->tm_min,
                         ltm->tm_sec);
                char folder[26];
                snprintf(folder, sizeof(folder),
                         "/home/data/img/%04d-%02d-%02d/",
                         1900 + ltm->tm_year,
                         1 + ltm->tm_mon,
                         ltm->tm_mday);
                std::filesystem::create_directories(folder);
                std::ofstream csvFile("/home/data/"+YMD+"-detect.csv", std::ios::app);
                if(!csvFile.is_open()) cerr << "open csv failed\n";

                printf("\r%s", time_f);
                fflush(stdout);
                Camera.grab();
                Camera.retrieve(frame);
                cv::Mat inputImg =
                    matPreprocess(frame, this->width(), this->height());

                // flatten rgb image to input layer.
                memcpy(this->input_tensor->data.f, inputImg.ptr<float>(0),
                       this->width()*this->height()*this->input_channels()*sizeof(float));

                this->interpreter->Invoke();

                vector<float> box_vec = cvtTensor(this->output_box);
                vector<float> score_vec = cvtTensor(this->output_score);

                vector<cv::Rect> result_box;
                if(yoloResult(box_vec, score_vec, 0.6, result_box)){
                    printf("\tThere are %d\n", result_box.size());
                    fflush(stdout);
                    std::filesystem::current_path(folder);
                    std::string file = time_f;
                    cv::imwrite(file+".jpg", frame);
                    csvFile << file << "," << std::to_string(result_box.size()) << '\n';
                    csvFile.flush();
                }
                else{
                    std::filesystem::current_path("/home/data/");
                    printf("\tNothing");
                    fflush(stdout);
                }

                // end = std::chrono::system_clock::now();
                // elapsed_seconds = end - start;
                // printf("\rs: %.5f", elapsed_seconds.count());
            }
            Camera.release();

            // cv::flip(frame, frame, -1);
            // cv::imwrite("raspicam_cv_image.jpg", frame);
            return true;
        }
        auto RunImage(std::string fileName){

        }

    private:
        uint height() const{
            return this->input_tensor->dims->data[1];
        }
        uint width() const{
            return this->input_tensor->dims->data[2];
        }
        uint input_channels() const{
            return this->input_tensor->dims->data[3];
        }
        std::unique_ptr<tflite::FlatBufferModel> model;
        std::unique_ptr<tflite::Interpreter> interpreter;
        TfLiteTensor* input_tensor = nullptr;
        TfLiteTensor* output_box = nullptr;
        TfLiteTensor* output_score = nullptr;
        int vW = 1280;
        int vH = 960;
};

int main(){
    if (system("CLS")) system("clear");
    CowMonitor cow_monitor;
    if(!cow_monitor.Init("yolov4-tiny-416-fp16.tflite"))
        cerr << "Stop!\n";
    if(!cow_monitor.Stream()) cerr << "Stop!\n";
    return 0;
}
