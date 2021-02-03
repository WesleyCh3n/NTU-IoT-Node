import cv2
import numpy as np


def imagePreproccess(filename):
    image = cv2.imread(filename).astype(np.float32)
    image = cv2.resize(image, (416, 416))/255.
    return image[:,:,::-1]

def cPreproccess(image):
    image = image.astype(np.float32)
    image = cv2.resize(((image/255.0)-0.5)*2.0, (224, 224))
    return [image]

def fLayerProcess(boxes_raw, scores_raw, thres):
    scores = scores_raw[np.where(scores_raw > thres)]
    sel_boxes = boxes_raw[np.where(scores_raw > thres)]
    xy = sel_boxes[:,:2]
    wh = sel_boxes[:,2:]
    boxes = np.c_[(xy-wh/2.)/416,(xy+wh/2.)/416]
    boxes = (boxes * [1280, 960, 1280, 960]).astype(int)
    return boxes, scores

def nms(boxes, scores, thresh):
    """Pure Python NMS baseline."""
    x1 = boxes[:, 0]
    y1 = boxes[:, 1]
    x2 = boxes[:, 2]
    y2 = boxes[:, 3]

    areas = (x2 - x1 + 1) * (y2 - y1 + 1)
    order = scores.argsort()[::-1]

    indices = []
    while order.size > 0:
        i = order[0]
        indices.append(i)
        xx1 = np.maximum(x1[i], x1[order[1:]])
        yy1 = np.maximum(y1[i], y1[order[1:]])
        xx2 = np.minimum(x2[i], x2[order[1:]])
        yy2 = np.minimum(y2[i], y2[order[1:]])

        w = np.maximum(0.0, xx2 - xx1 + 1)
        h = np.maximum(0.0, yy2 - yy1 + 1)
        inter = w * h
        ovr = inter / (areas[i] + areas[order[1:]] - inter)

        inds = np.where(ovr <= thresh)[0]
        order = order[inds + 1]
    return indices
