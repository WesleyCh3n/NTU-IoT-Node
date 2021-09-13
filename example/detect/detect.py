import cv2
import time
import numpy as np

width = 256
weight = f"./CowFace-{width}_6000.weights"
config = f"./CowFace-{width}.cfg"
size = (width,width)

net = cv2.dnn.readNet(weight, config)
model = cv2.dnn_DetectionModel(net)
model.setInputParams(size=size,scale=1./255,swapRB=True)

img = cv2.imread("detect.jpg")
out_img = np.copy(img)

for i in range(0,11):
    start = time.time()
    classes, scores, boxes = model.detect(img, 0.75, 0.4)
    end = time.time()
    print("{:.6f}".format((end - start)*1000))

    for (classid, score, box) in zip(classes, scores, boxes):
        cv2.rectangle(out_img, box, (0,0,255), 2)
        cv2.imwrite("./out.jpg", out_img)
