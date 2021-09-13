#include "network/network.h"

/*
 * Preprocess input cv::Mat image
 * @param
 *     [out] src: input image
 *     [in] width: model input width
 *     [in] height: model input height
 *     [in] *norm(pixel &): pointer of function of normalization take pixel as
 *                          reference input
 * @return
 *     preprocessed image
 *  */
cv::Mat matPreprocess(cv::Mat &src, uint width, uint height,
                      void (*norm)(Pixel&)){
    // convert to float; BGR -> RGB
    cv::Mat dst;
    src.convertTo(dst, CV_32FC3);
    cv::cvtColor(dst, dst, cv::COLOR_BGR2RGB);

    // normalize
    Pixel* pixel = dst.ptr<Pixel>(0,0);
    const Pixel* endPixel = pixel + dst.cols * dst.rows;
    for (; pixel != endPixel; pixel++)
        norm(*pixel);

    // resize image as model input
    cv::resize(dst, dst, cv::Size(width, height));
    return dst;
}

std::vector<float> cvtTensor(TfLiteTensor* tensor){
    int nelem = 1;
    for(int i=0; i<tensor->dims->size; ++i) //length of dim. ex: (1,4,4) -> 3
        nelem *= tensor->dims->data[i]; //dim number. ex: (1,3,3) -> nelem=9
    std::vector<float> data(tensor->data.f, tensor->data.f+nelem);
    return data;
}

void yolov4_norm(Pixel &pixel){
    pixel.x = (pixel.x / 255.0);
    pixel.y = (pixel.y / 255.0);
    pixel.z = (pixel.z / 255.0);
}

void mobilenetv2_norm(Pixel &pixel){
    pixel.x = ((pixel.x / 255.0)-0.5)*2.0;
    pixel.y = ((pixel.y / 255.0)-0.5)*2.0;
    pixel.z = ((pixel.z / 255.0)-0.5)*2.0;
}
