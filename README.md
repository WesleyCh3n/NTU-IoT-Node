# NTU-IOT-NODE

This is my NTU MS thesis project. The purpose of this project is using edge
 device (Rpi) to monitoring dairy cow feeding behaviour. The code is focus on
 streaming also recognizing individual cow faces by edge computing (with
 Tensorflow Lite etc).

This repo contains c++ and python version. The python version is deprecated, but still
usable. The main program uses c++ which is in 
[cc/recognition](https://github.com/WesleyCh3n/ntu-iot-node/tree/main/cc/recognition)
 folder.

## Usage

```bash
$ ./ntu-iot-node -h
Cow face monitoring system
Usage:
  ntu-iot-node [OPTION...]

  -s, --stream        Start streaming
  -i, --image arg     Recognize Image
  -m, --mode arg      mode: 0:detect 1:classify 2:recognize (default: 2)
  -D, --detect arg    Detect model path (default:
                      ./model/yolov4-tiny-416-fp16.tflite)
  -C, --classify arg  Classify model path (default:
                      ./model/mobilenetv2-128.tflite)
  -R, --ref arg       class reference (default: ./19-01.tsv)
  -h, --help          Print usage

```
