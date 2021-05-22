#include "yolov4-tiny.h"
#include "iostream"

/*
 * Allocate yolo model to tensor
 * @param
 *     [in] model_path: model path
 *     [in] conf_thres: confidence threshold
 *     [in] nms_thres: non-maximum suppression threshold
 *     [in] img_w: original image width
 *     [in] img_h: original image height
 * @return
 *     status
 *  */
Yolov4Tiny::Yolov4Tiny(std::string model_path,
                       float conf_thres, float nms_thres,
                       int img_w, int img_h):
                       W_(img_w), H_(img_h),
                       conf_thres_(conf_thres), nms_thres_(nms_thres){
    /* allocate model tensor */
    model_= tflite::FlatBufferModel::BuildFromFile(model_path.c_str());
    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder(*model_.get(), resolver)(&interpreter_);
    interpreter_->AllocateTensors();

    /* get input and output tensor */
    inp_tensor_ = interpreter_->tensor(interpreter_->inputs()[0]);
    out_tensor_box_ = interpreter_->tensor(interpreter_->outputs()[0]);
    out_tensor_score_ = interpreter_->tensor(interpreter_->outputs()[1]);

    /* get input tensor size */
    /* input_dim_.h = inp_tensor_->dims->data[1];
     * input_dim_.w = inp_tensor_->dims->data[2];
     * input_dim_.c = inp_tensor_->dims->data[3]; */

    /* log information */
    std::cout << std::left
        << std::setw(16) << "Detect model" << ": " << model_path << '\n'
        << std::setw(16) << "Image size" << ": " << W_ << " " << H_ << '\n'
        << std::setw(16) << "Input size" << ": "
        << inp_tensor_->dims->data[1] << " "
        << inp_tensor_->dims->data[2] << " "
        << inp_tensor_->dims->data[3] << '\n'
        << std::setw(16) << "Conf thres" << ": " << conf_thres_ << '\n'
        << std::setw(16) << "NMS thres" << ": " << nms_thres_ << '\n';
}

/*
 * Find valid bbox by confidence and non-maximum suppression
 * @param
 *     [in] box: raw result of yolo bbox
 *     [in] score: raw result of yolo score
 *     [out] result: valid bbox with cv::Rect(xmin,xmax,w,h) as vector
 * @return
 *     status: if no valid bbox return false else true
 *  */
bool Yolov4Tiny::postprocess(std::vector<float> &box, std::vector<float> &score,
                             std::vector<cv::Rect> &result){
    auto it = std::find_if(std::begin(score), std::end(score),
                           [this](float i){return i > conf_thres_;});
    std::vector<cv::Rect> sel_rects;
    std::vector<float> sel_score;
    cv::Rect roiImg(0, 0, W_, H_);
    while (it != std::end(score)) {
        size_t id      = std::distance(std::begin(score), it);
        const int cx   = box[4*id];
        const int cy   = box[4*id+1];
        const int w    = box[4*id+2];
        const int h    = box[4*id+3];
        const int xmin = ((cx-(w/2.f))/inp_tensor_->dims->data[2]) * W_;
        const int ymin = ((cy-(h/2.f))/inp_tensor_->dims->data[1]) * H_;
        const int xmax = ((cx+(w/2.f))/inp_tensor_->dims->data[2]) * W_;
        const int ymax = ((cy+(h/2.f))/inp_tensor_->dims->data[1]) * H_;
        sel_rects.emplace_back(cv::Rect(xmin, ymin, xmax-xmin, ymax-ymin) & roiImg);
        sel_score.emplace_back(score[id]);
        it = std::find_if(std::next(it), std::end(score),
                          [this](float i){return i > conf_thres_;});
    }
    if(sel_rects.empty())
        return false;
    std::vector<int> sel_index;
    cv::dnn::NMSBoxes(sel_rects, sel_score, conf_thres_, nms_thres_, sel_index);
    if(sel_index.empty())
        return false;
    for(int &i: sel_index){
        result.emplace_back(sel_rects[i]);
    }
    return true;
}

/*
 * Compute yolo model and get the output
 * @param
 *     [in] inputImg: input image
 *     [out] result_box: result bounding box of input image
 * @return
 *     status: if no valid bbox return false else true
 *  */
bool Yolov4Tiny::invoke(cv::Mat inputImg, std::vector<cv::Rect> &result_box){
    result_box.clear();
    inputImg = matPreprocess(inputImg,
                             inp_tensor_->dims->data[2],
                             inp_tensor_->dims->data[1],
                             yolov4_norm);
    memcpy(inp_tensor_->data.f, inputImg.ptr<float>(0),
           inp_tensor_->dims->data[1] * inp_tensor_->dims->data[2] *
           inp_tensor_->dims->data[3] * sizeof(float));
    interpreter_->Invoke();
    std::vector<float> box_vec   = cvtTensor(out_tensor_box_);
    std::vector<float> score_vec = cvtTensor(out_tensor_score_);
    return postprocess(box_vec, score_vec, result_box);
}

