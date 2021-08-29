# NTU-IOT-NODE

[![hackmd-github-sync-badge](https://hackmd.io/V8NktJtNSamroQN6Tfxl5A/badge)](https://hackmd.io/V8NktJtNSamroQN6Tfxl5A)
[![WesleyCh3n - ntu-IoT-Node](https://img.shields.io/badge/WesleyCh3n-ntu--IoT--Node-2ea44f?logo=github)](https://github.com/WesleyCh3n/ntu-IoT-node)


This is NTU BME MS thesis project. The purpose of this project is using edge
device (Rpi) to monitoring dairy cow feeding behavior. The code is focus on
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

All libraries are built in static and default location is placed in `/opt/`. Or, modify
[Makefile](https://github.com/WesleyCh3n/ntu-iot-node/blob/main/cc/recognition/Makefile) (L15-L22)
 to the correct folder you built.



| Library                                          |                                      Pre-built Link                                      |
|:------------------------------------------------ |:----------------------------------------------------------------------------------------:|
| [OpenCV](https://bit.ly/2Y8KyJK): 4.5.1          | [✔️](https://drive.google.com/file/d/1-HfoQ7DWhVF3rPNbOTsw3kopCuwh7h2H/view?usp=sharing) |
| [Tensorflow Lite](https://bit.ly/3ytc6Wu)        | [✔️](https://drive.google.com/file/d/1RcNIygC6bi8E5EsYmM1a2YKjioC5iFZE/view?usp=sharing) |
| [RaspiCam](https://bit.ly/38mTsFl)               | [✔️](https://drive.google.com/file/d/1p4DOahgOeoG4AQ29AB3A_YiUF7fMxFxb/view?usp=sharing) |
| [CXXopts](https://bit.ly/3sU28MO): 2.2.1         | [✔️](https://drive.google.com/file/d/1VU4EA80AE_xNnJDVePAJEUNmqPtHWcps/view?usp=sharing) |
| [MQTT](https://github.com/eclipse/paho.mqtt.cpp) | [✔️](https://drive.google.com/file/d/1BOVi3j5v8offJPDaFkm6jIUpn9Gw38va/view?usp=sharing) |
| [Boost](https://bit.ly/2UX4A8J): 1.75.0          | [✔️](https://drive.google.com/file/d/1IJhaDof-paWjeXAZWeOmyLD-co-j-6Vs/view?usp=sharing) |

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