#include "mobilenetv2.h"
#include <iostream>

/*
 * Allocate classification model to tensor
 * @param
 *     [in] model_path: model path
 * @return
 *     status
 *  */
MobileNetv2::MobileNetv2(std::string model_path){
    model_= tflite::FlatBufferModel::BuildFromFile(model_path.c_str());
    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder(*model_.get(), resolver)(&interpreter_);
    interpreter_->AllocateTensors();

    inp_tensor_ = interpreter_->tensor(interpreter_->inputs()[0]);
    out_tensor_ = interpreter_->tensor(interpreter_->outputs()[0]);

    /* log information */
    std::cout << std::left
        << std::setw(16) << "Classify Model" << ": " << model_path << '\n'
        << std::setw(16) << "Input size" << ": "
        << inp_tensor_->dims->data[1] << " "
        << inp_tensor_->dims->data[2] << " "
        << inp_tensor_->dims->data[3] << '\n';
}

/*
 * Compute MobileNetv2 model and return output (128 dim)
 * @param
 *     [in] inputImg: input image
 *     [out] result: vector of feature (128)
 *  */
 std::vector<float> MobileNetv2::invoke(cv::Mat inputImg){
    inputImg = matPreprocess(inputImg,
                             inp_tensor_->dims->data[2],
                             inp_tensor_->dims->data[1],
                             mobilenetv2_norm);
    memcpy(inp_tensor_->data.f, inputImg.ptr<float>(0),
           inp_tensor_->dims->data[1] * inp_tensor_->dims->data[2] *
           inp_tensor_->dims->data[3] * sizeof(float));
    interpreter_->Invoke();
    return cvtTensor(out_tensor_);
}

