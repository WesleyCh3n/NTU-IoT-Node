#!/usr/bin/python3
# -*- coding: utf-8 -*-

from picamera import PiCamera
import numpy as np
import time
import cv2
from tflite_runtime.interpreter import Interpreter
from core.utils import *




if __name__ == "__main__":
    refs = np.loadtxt("./dataset/19-01.tsv", dtype=np.float16, delimiter='\t')

    yoloPath = "./model/yolov4-tiny-416-fp16.tflite"
    dModel = Interpreter(model_path=yoloPath)
    dModel.allocate_tensors()

    mobilePath = "./model/mobilenetv2-128.tflite"
    cModel = Interpreter(model_path=mobilePath)
    cModel.allocate_tensors()

    dInput = dModel.get_input_details()[0]['index']
    dOutputBoxes = dModel.get_output_details()[0]["index"]
    dOutputScores = dModel.get_output_details()[1]["index"]
    cInput = cModel.get_input_details()[0]["index"]
    cOutput = cModel.get_output_details()[0]["index"]

    with PiCamera() as camera:
        # Setting up camera
        camera.resolution = (1280, 960)
        camera.start_preview()
        # camera.vflip = True
        # time.sleep(1)

        i = 0
        while True:
            # pre allocate img space
            output = np.empty((960, 1280, 3), dtype=np.uint8)

            # Start capturing i:index f:frame
            stream = camera.capture_continuous(output, format="rgb")
            for(i, img) in enumerate(stream):
                f = img.copy()
                # create file name and save img folder in docker container
                s = time.time()
                f = cv2.resize(f.astype(np.float32), (416,416))/255.
                dModel.set_tensor(dInput, [f])
                dModel.invoke()
                boxes_raw = dModel.get_tensor(dOutputBoxes)[0]
                scores_raw = dModel.get_tensor(dOutputScores).flatten()
                boxes, scores = fLayerProcess(boxes_raw, scores_raw, 0.6)
                if len(boxes) > 0:
                    indices = np.array(cv2.dnn.NMSBoxes(boxes.tolist(),
                                                        scores.tolist(),
                                                        0.6, 0.4)).flatten()
                    vecs = []
                    for box in boxes[indices]:
                        # cv2.rectangle(img, (box[0], box[1]), (box[2], box[3]), (0, 0, 255), 2)
                        cModel.set_tensor(cInput, 
                                          cPreproccess(img[box[1]:box[3], box[0]:box[2]]))
                        cModel.invoke()
                        output_data = cModel.get_tensor(cOutput)[0]
                        vecs.append(output_data)
                    vecs = np.array(vecs)
                    dists = np.argmin(-2*np.dot(vecs, refs.T)+
                                      np.sum(refs**2, axis=1)+
                                      np.sum(vecs**2, axis=1)[:,np.newaxis],
                                      axis=1)
                    print(dists)
                else:
                    print("nothing")
                e = time.time()
                print(f"Spend: {e-s:.3f}")
