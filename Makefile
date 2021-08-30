CC := arm-linux-gnueabihf-g++
CCFLAGS := -Bstatic -std=c++17 -O3
SRC := main.cc src/cow_monitor.cpp
TARGET := ntu-iot-node
OBJFILES := cow_monitor.o main.o

LIBFILES := ./src/network/lib/libnetwork.a \
./src/yolov4-tiny/lib/libyolov4_tiny.a \
./src/mobilenetv2/lib/libmobilenetv2.a

ifeq ($(RELEASE),1)
	CCFLAGS += -DRELEASE -DNTU_IOT_NODE_VERSION=\"$(VERSION)\"
endif

NETWORK_DIR := ./src/network
YOLO_DIR := ./src/yolov4-tiny
MOBILENET_DIR = ./src/mobilenetv2

TF_DIR := /opt/tensorflow_src
OPENCV_DIR := /opt/opencv
RASPICAM_DIR := /opt/raspicam
VC_DIR := /opt/vc
BOOST_DIR := /opt/boost_1_75_0
CXXOPT_DIR := /opt/cxxopts-2.2.1
MQTTC_DIR := /opt/paho.mqtt.c
MQTTCXX_DIR := /opt/paho.mqtt.cpp

INCLUDES := \
-I$(TF_DIR)/tensorflow/lite/tools/make/downloads/flatbuffers/include \
-I$(TF_DIR) \
-I$(OPENCV_DIR)/include/opencv4 \
-I$(RASPICAM_DIR)/include \
-I$(BOOST_DIR) \
-I$(CXXOPT_DIR)/include \
-I$(MQTTC_DIR)/build/install/include \
-I$(MQTTCXX_DIR)/build/install/include \
-I$(NETWORK_DIR) \
-I.

LIBDIRS := \
-L$(TF_DIR)/tensorflow/lite/tools/make/gen/rpi_armv7l/lib \
-L$(OPENCV_DIR)/lib/ \
-L$(OPENCV_DIR)/lib/opencv4/3rdparty \
-L$(OPENCV_DIR)/build/lib \
-L$(RASPICAM_DIR)/lib \
-L$(MQTTC_DIR)/build/install/lib \
-L$(MQTTCXX_DIR)/build/install/lib \
-L$(VC_DIR)/lib/ \
-L$(NETWORK_DIR)/lib/ \
-L$(YOLO_DIR)/lib \
-L$(MOBILENET_DIR)/lib \

LIBS := \
-lstdc++fs \
-lyolov4_tiny \
-lmobilenetv2 \
-lnetwork \
-ltensorflow-lite \
`pkg-config --libs-only-l opencv4` \
-littnotify -llibprotobuf -llibwebp \
-llibopenjp2 -lIlmImf -lquirc -ltegra_hal \
-lade -ljpeg -lpng -ltiff \
-lpaho-mqttpp3 -lpaho-mqtt3as -lpaho-mqtt3cs \
-lz -lm -lpthread -ldl -lrt \
-lraspicam -lraspicam_cv -lmmal -lmmal_core -lmmal_util \
-Wl,-Bstatic -lssl -lcrypto -Wl,-Bdynamic

# make clean;make RELEASE=1 VERSION=2.0.1
all:$(TARGET)

$(TARGET):$(OBJFILES)
	@echo "CC $@ FROM $^"
	@$(CC) $(CCFLAGS) -o $(TARGET) $(OBJFILES) $(INCLUDES) $(LIBDIRS) $(LIBS)
	@echo "Compiling Complete"

$(OBJFILES):$(SRC) $(LIBFILES)
	@echo "Compiling VERSION: $(VERSION)"
	@echo "CC $@ FROM $^"
	@$(CC) $(CCFLAGS) -c $(SRC) $(INCLUDES) $(LIBDIRS) $(LIBS)

$(LIBFILES):
	@echo "Compiling ./src/network/network.cc"
	@make -C ./src/network/
	@echo "Compiling ./src/yolov4-tiny/yolov4-tiny.cc"
	@make -C ./src/yolov4-tiny/
	@echo "Compiling ./src/mobilenetv2/mobilenetv2.cc"
	@make -C ./src/mobilenetv2/

.PHONY: clean
clean:
	rm -f $(TARGET) cow_monitor.o main.o
