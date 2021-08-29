# NTU-IOT-NODE

[![hackmd-github-sync-badge](https://hackmd.io/V8NktJtNSamroQN6Tfxl5A/badge)](https://hackmd.io/V8NktJtNSamroQN6Tfxl5A)


This is NTU BME MS thesis project. The purpose of this project is using edge
device (Rpi) to monitoring dairy cow feeding behaviour. The code is focus on
streaming also recognizing individual cow faces by edge computing (with
Tensorflow Lite etc).

This repo contains c++ and python version. *The python version is deprecated, but still
usable*(in `python` branch). The main program uses c++ which is in
[cc/recognition](https://github.com/WesleyCh3n/ntu-iot-node/tree/main/cc/recognition)
 folder.

## Usage

```bash
$ ./ntu-iot-node -h
Cow face monitoring system
Usage:
  ntu-iot-node [OPTION...]

  -s, --stream        Start streaming
  -n, --node arg      node number (default: )
  -i, --image arg     Recognize Image
  -m, --mode arg      mode: 0:detect 1:classify 2:recognize (default: 2)
  -D, --detect arg    Detect model path (default:
                      ./model/yolov4-tiny-416-fp16.tflite)
  -C, --classify arg  Classify model path (default:
                      ./model/mobilenetv2-128.tflite)
  -R, --ref arg       class reference (default: ./19-01.tsv)
  -q, --mqtt arg      mqtt address
  -u, --user arg      mqtt username
  -p, --pwd arg       mqtt password
  -v, --version       ntu-iot-node version
  -h, --help          Print usage
```

## Build

### Platform

- os: Raspbian GNU/Linux 10 (buster)
- kernel: 5.4.51-v7l+

### Prerequisites

All libraries are built in static and default location is in `/opt/`. Or, modify
[Makefile](https://github.com/WesleyCh3n/ntu-iot-node/blob/main/cc/recognition/Makefile)
 to the correct folder you built.

- [OpenCV](https://github.com/opencv/opencv): 4.5.1
- [Tensorflow Lite](https://www.tensorflow.org/lite/guide/build_rpi)
- [RaspiCam](https://github.com/cedricve/raspicam)
- [Boost](https://www.boost.org/): 1.75.0
- [CXXopts](https://github.com/jarro2783/cxxopts): 2.2.1

**Pre-built Library**: [https://drive.google.com/file/d/1sUfFn1Snn9hA2zla_V28DwZs32q0pdnA/view?usp=sharing]

go to `/opt/` and extract all then good to go.

### Compiling

Clone the project to the Raspberry Pi. And cd to cc/recognition/ then compile.
Make sure your prerequisites are built.
```bash
git clone https://github.com/WesleyCh3n/ntu-iot-node
cd cc/recognition/
make clean; make RELEASE=1 VERSION=<version number>
```
you will see something like
```bash
rm -f ntu-iot-node include/cow_monitor.o main.o
Compiling cow_monitor.o
Compiling ntu-iot-node
Compiling Complete
```
then can type `./ntu-iot-node -h` to see the usage.

### Docker testing

After compiling complete, you can use this docekr image to verify
```bash
$ docker run -it --rm --privileged=true -w /home/ -v `pwd`:/home/ -v /opt/vc:/opt/vc --device=/dev/vchiq --device=/dev/vcsm cpp-slim bash
root@CONTAINER_ID:/home# ./ntu-iot-node -h
```