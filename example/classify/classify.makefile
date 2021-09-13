CC := arm-linux-gnueabihf-g++
CCFLAGS := -Bstatic -std=c++17 -O3
SRC := ./classify.cc
TARGET := ./classify

MOBILENET_DIR = ../src/mobilenetv2
NETWORK_DIR := ../src/network
OPENCV_DIR := /opt/opencv
TF_DIR := /opt/tensorflow_src

INCLUDES := \
-I$(OPENCV_DIR)/include/opencv4 \
-I$(TF_DIR)/tensorflow/lite/tools/make/downloads/flatbuffers/include \
-I$(TF_DIR) \
-I$(NETWORK_DIR) \
-I$(MOBILENET_DIR) \
-I. \

LIBDIRS := \
-L$(TF_DIR)/tensorflow/lite/tools/make/gen/rpi_armv7l/lib \
-L$(OPENCV_DIR)/lib/ \
-L$(OPENCV_DIR)/lib/opencv4/3rdparty \
-L$(OPENCV_DIR)/build/lib \
-L$(NETWORK_DIR)/lib/ \
-L$(MOBILENET_DIR)/lib \

LIBS := \
-lstdc++fs \
-lmobilenetv2 \
-lnetwork \
-ltensorflow-lite \
`pkg-config --libs-only-l opencv4` \
-littnotify -llibprotobuf -llibwebp \
-llibopenjp2 -lIlmImf -lquirc \
-ltegra_hal -lade -ljpeg -lpng -ltiff \
-lz -lm -lpthread -ldl -lrt \
-Wl,-Bstatic -lssl -lcrypto -Wl,-Bdynamic

all:$(TARGET)

$(TARGET):$(SRC)
	@echo "CC $@"
	@$(CC) $(CCFLAGS) -o $(TARGET) $(SRC) $(INCLUDES) $(LIBDIRS) $(LIBS)

.PHONY: clean
clean:
	@echo "Clean ..."
	@rm -f $(TARGET)
