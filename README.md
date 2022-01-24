# NTU-IoT-Node

[![WesleyCh3n - ntu-IoT-node](https://img.shields.io/badge/WesleyCh3n-NTU--IoT--Node-2ea44f?style=for-the-badge&logo=github)](https://github.com/WesleyCh3n/NTU-IoT-Node)
![Docker - cross-compile](https://img.shields.io/badge/Docker-cross--compile-informational?style=for-the-badge&logo=docker)
![OS - Raspberry Pi](https://img.shields.io/badge/OS-Raspberry_Pi-blue?style=for-the-badge&logo=Raspberry+Pi)
![Made with - C++](https://img.shields.io/badge/Made_with-C%2B%2B17-informational?style=for-the-badge&logo=C%2B%2B)
![TFLite - 2.4.1](https://img.shields.io/badge/TFLite-2.4.1-informational?style=for-the-badge&logo=Tensorflow)
![Bash - 5.0.3](https://img.shields.io/badge/Bash-5.0.3-informational?style=for-the-badge&logo=gnu-bash)


This is NTU BME MS thesis project. The purpose of this project is using edge
device (Rpi) to monitoring dairy cow feeding behavior. The code is focus on
streaming also recognizing individual cow faces by edge computing (with
Tensorflow Lite etc).

This repo contains c++ and python version. *The python version is deprecated, but still
usable*(in `python` branch). The main program uses c++.

## Usage

```bash
$ ntu-node -h
NTU BBLAB cow face monitoring system
Usage:
  ntu-node [OPTION...]

  -s, --stream        Start streaming
  -i, --image arg     image directory to recognize (default: )
  -D, --detect arg    detect model path (default:
                      ./model/yolov4-tiny-f16.tflite)
  -C, --classify arg  classify model path (default:
                      ./model/mobilenetv2.tflite)
  -n, --node arg      node number (default: )
  -r, --ref arg       class reference (default: ./cfg/ref.8f.tsv)
  -d, --ref_dict arg  class id (default: ./cfg/ref_dict.csv)
  -f, --fence arg     fence bbox (default: )
  -m, --mqtt_ip arg   mqtt broker ip (default: )
  -u, --user arg      mqtt username (default: )
  -p, --pwd arg       mqtt password (default: )
  -v, --version       ntu-node version
  -h, --help          Print usage
```

## Build

### Platform

- os: Raspbian GNU/Linux 10 (buster)
- kernel: 5.4.51-v7l+ / 5.10.60-v8+

### Prerequisites (third-party library)

All libraries are built in static and default location is placed in `/opt/`. Or, modify
`CMakeLists.txt` to the correct folder you built.

| Library                                                 | armv7                                                                                        | arm64                                                                                        |
|:--------------------------------------------------------|:--------------------------------------------------------------------------------------------:|:--------------------------------------------------------------------------------------------:|
| [OpenCV](https://bit.ly/2Y8KyJK): 4.5.3                 | [✔️](https://github.com/WesleyCh3n/NTU-IoT-Node/blob/main/3rdparty/armv7/download_opencv.sh)  | [✔️](https://github.com/WesleyCh3n/NTU-IoT-Node/blob/main/3rdparty/arm64/download_opencv.sh)  |
| [Tensorflow Lite](https://bit.ly/3ytc6Wu): 2.4.2        | [✔️](https://github.com/WesleyCh3n/NTU-IoT-Node/blob/main/3rdparty/armv7/download_tflite.sh)  | [✔️](https://github.com/WesleyCh3n/NTU-IoT-Node/blob/main/3rdparty/arm64/download_tflite.sh)  |
| [CXXopts](https://bit.ly/3sU28MO): 2.2.1                | [✔️](https://github.com/WesleyCh3n/NTU-IoT-Node/blob/main/3rdparty/armv7/download_cxxopts.sh) | [✔️](https://github.com/WesleyCh3n/NTU-IoT-Node/blob/main/3rdparty/arm64/download_cxxopts.sh) |
| [MQTT](https://github.com/eclipse/paho.mqtt.cpp): 1.2.0 | [✔️](https://github.com/WesleyCh3n/NTU-IoT-Node/blob/main/3rdparty/armv7/download_mqtt.sh)    | [✔️](https://github.com/WesleyCh3n/NTU-IoT-Node/blob/main/3rdparty/arm64/download_mqtt.sh)    |
| [Boost](https://bit.ly/2UX4A8J): 1.75.0                 | [✔️](https://github.com/WesleyCh3n/NTU-IoT-Node/blob/main/3rdparty/armv7/download_boost.sh)   | [✔️](https://github.com/WesleyCh3n/NTU-IoT-Node/blob/main/3rdparty/arm64/download_boost.sh)   |

use `download_xxx.sh` to download and extract.

For OpenCV, extract and cp `opencv/lib/pkgconfig/opencv4.pc` to your `$PKG_CONFIG_PATH` (default: `/usr/lib/pkgconfig/`). Then type `pkg-config --list-all|grep opencv` and if you see

```bash=!
$ pkg-config --list-all|grep opencv
opencv4               OpenCV - Open Source Computer Vision Library
```
OpenCV is set up.

Finally, go to `/opt/` and extract all then good to go.

### RPi Native Compiling

Clone the project to the Raspberry Pi. And cd to cc/recognition/ then compile.
Make sure your prerequisites are built.
```bash
git clone https://github.com/WesleyCh3n/NTU-IoT-Node && cd NTU-IoT-Node
mkdir build && cd build
cmake ../src/
```
you will see something like
```bash
-- The C compiler identification is GNU 8.3.0
-- The CXX compiler identification is GNU 8.3.0
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/local/aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/local/aarch64-linux-gnu/bin/aarch64-linux-gnu-g++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Found PkgConfig: /usr/bin/pkg-config (found version "0.29.2")
-- Checking for module 'opencv4'
--   Found opencv4, version 4.5.0
-- NTU IOT NODE:
--   VERSION = beta
-- FOUND ARCH: arm64v8
-- Configuring done
-- Generating done
-- Build files have been written to: /work/arm64/ntu-node
```
make sure `ARCH` is the architecture you want to perform (in this example, aarch64 is built)

After cmake complete, start compile by (-j<number of thread>)

```bash
make -j4
```
once finished, you will get following output
```bash
[ 20%] Building CXX object cow_monitor/yolov4-tiny_lib/CMakeFiles/yolov4-tiny.dir/yolov4-tiny.cc.o
[ 20%] Building CXX object cow_monitor/network_lib/CMakeFiles/network.dir/network.cc.o
[ 30%] Building CXX object cow_monitor/mobilenetv2_lib/CMakeFiles/mobilenetv2.dir/mobilenetv2.cc.o
[ 40%] Linking CXX static library libnetwork.a
[ 40%] Built target network
[ 50%] Linking CXX static library libmobilenetv2.a
[ 50%] Built target mobilenetv2
[ 60%] Linking CXX static library libyolov4-tiny.a
[ 60%] Built target yolov4-tiny
[ 70%] Building CXX object cow_monitor/CMakeFiles/cow_monitor.dir/cow_monitor.cpp.o
[ 80%] Linking CXX static library libcow_monitor.a
[ 80%] Built target cow_monitor
[ 90%] Building CXX object CMakeFiles/ntu-node.dir/main.cc.o
[100%] Linking CXX executable ntu-node
[100%] Built target ntu-node
```

Then the folder should have `ntu-node` binary.

### Cross Compiling

Use [`pi-cross`](https://github.com/WesleyCh3n/pi-cross) docker image to build. Follow `pi-cross` instruction to install helper script on your x64 host machine. Then,

```bash
git clone https://github.com/WesleyCh3n/NTU-IoT-Node && cd NTU-IoT-Node
mkdir -p ./build/opt && cd build/opt/

# download all lib
../../3rdparty/armv7/download_mqtt.sh
../../3rdparty/armv7/download_boost.sh
../../3rdparty/armv7/download_opencv.sh
../../3rdparty/armv7/download_tflite.sh
../../3rdparty/armv7/download_cxxopts.sh

# Start pi-cross container instance
cd ../../ # Back to root dir of NTU-IoT-Node
# Start container with mounting opencv.pc and lib folder
armv7-8.3.0 -a "-v `pwd`/build/opt/opencv/lib/pkgconfig/opencv4.pc:/usr/lib/pkgconfig/opencv4.pc:ro \
-v `pwd`/build/opt:/opt/" bash

# (following is in container)
cd build && cmake ../src/
make -j4

# Then the `build` directory should have `ntu-node` binary.
exit
```

### Rpi Native Docker Testing

After compiling complete, you can use the docker image to verify on RPi.

For arm32v7: [docker image](https://hub.docker.com/layers/arm32v7/debian/stable-slim/images/sha256-7bb9de2067f4e4e3e2377070e180a05d33a0bc4289c66b9e71504063cf18da15?context=explore)
```bash
$ docker run -it --rm --privileged=true \
-w /home/ \
-v `pwd`:/home/ \
--device=/dev/video0
arm32v7/debian:stable-slim bash
```

For arm64v8: [docker image](https://hub.docker.com/layers/arm64v8/debian/stable-slim/images/sha256-051adf1212e6a3ba39a13a02afd690a81e6422461b81042c35c74ae9cc8ed272?context=explore)
```bash
$ docker run -it --rm --privileged=true \
-w /home/ \
-v `pwd`:/home/ \
--device=/dev/video0
arm64v8/debian:stable-slim bash
```

Then
```bash
./ntu-node -h
```

---
## `upload.sh`

### Usage

```bash
$ ./upload.sh \
    -u rsync_user \
    -p rsync_ip \
    -P rsync_port \
    -d remote_folder \ # must have NODE+02d in the path to know which node is it
    -t backup_time_interval(s)
```

for example
```
$ ./upload.sh \
    -u root \
    -p 255.255.255.255 \
    -P 22 \
    -d /data/dir/NODE00/ \
    -t 600
```
