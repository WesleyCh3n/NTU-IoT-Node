#!/usr/bin/python3
# -*- coding: utf-8 -*-

from tflite_runtime.interpreter import Interpreter
from picamera import PiCamera
from pathlib import Path
import numpy as np
import time
import cv2
import os
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

        print("Start Detection")
        i = 0
        while True:
            # pre allocate img space
            output = np.empty((960, 1280, 3), dtype=np.uint8)

            # Start capturing i:index f:frame
            stream = camera.capture_continuous(output, format="rgb")
            for(i, img) in enumerate(stream):
                # create file name and save img folder in docker container
                fileTime = time.strftime("%Y_%m_%d-%H_%M_%S")
                Path(f"/home/img/{time.strftime('%Y-%m-%d')}/").mkdir(parents=True, exist_ok=True)
                path = os.path.join(f"/home/img/{time.strftime('%Y-%m-%d')}/", fileTime+'.jpg')

                f = img.copy()
                f = cv2.resize(f.astype(np.float32), (416,416))/255.
                dModel.set_tensor(dInput, [f])
                dModel.invoke()
                boxes_raw = dModel.get_tensor(dOutputBoxes)[0]
                scores_raw = dModel.get_tensor(dOutputScores).flatten()
                boxes, scores = fLayerProcess(boxes_raw, scores_raw, 0.6)
                if len(boxes) > 0:
                    print(f"{fileTime} - Number: {len(boxes)}")
