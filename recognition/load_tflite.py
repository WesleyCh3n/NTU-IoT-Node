#!/usr/bin/python3
# -*- coding: utf-8 -*-

import cv2
import numpy as np
import time
from tflite_runtime.interpreter import Interpreter
from core.utils import *


if __name__ == "__main__":
    refs = np.loadtxt("refs.tsv", dtype=np.float16, delimiter='\t')
    print(type(refs[0][0]))

    yoloPath = "./yolov4-tiny-416-fp16.tflite"
    dModel = Interpreter(model_path=yoloPath)
    dModel.allocate_tensors()

    mobilePath = "./model.tflite"
    cModel = Interpreter(model_path=mobilePath)
    cModel.allocate_tensors()

    imagePath="./NODE29.jpg"
    img = cv2.imread(imagePath)
    cvImg = imagePreproccess(imagePath)
    # get model layer info
    dInput = dModel.get_input_details()[0]['index']
    dOutputBoxes = dModel.get_output_details()[0]["index"]
    dOutputScores = dModel.get_output_details()[1]["index"]
    cInput = cModel.get_input_details()[0]["index"]
    cOutput = cModel.get_output_details()[0]["index"]

    dModel.set_tensor(dInput, [cvImg])
    s = time.time()
    dModel.invoke()
    boxes_raw = dModel.get_tensor(dOutputBoxes)[0]
    scores_raw = dModel.get_tensor(dOutputScores).flatten()
    boxes, scores = fLayerProcess(boxes_raw, scores_raw, 0.25)
    indices = np.array(cv2.dnn.NMSBoxes(boxes.tolist(), 
                                        scores.tolist(), 
                                        0.25, 0.35)).flatten()
    vecs = []
    for i, box in enumerate(boxes[indices]):
        cModel.set_tensor(cInput, 
                          cPreproccess(img[box[1]:box[3], box[0]:box[2]]))
        cModel.invoke()
        output_data = cModel.get_tensor(cOutput)[0]
        vecs.append(output_data)
        # print(output_data)
        # cv2.imwrite(f"./{i:02d}.jpg", img[box[1]:box[3], box[0]:box[2]])
        # cv2.rectangle(img, (box[0], box[1]), (box[2], box[3]), (0, 0, 255), 2)
    vecs = np.array(vecs)
    dists = -2*np.dot(vecs, refs.T)+np.sum(refs**2, axis=1)+np.sum(vecs**2, axis=1)[:,np.newaxis]
    print(np.argmin(dists, axis=1))
    e = time.time()
    print(f"Spend: {(e-s):.5f}")
