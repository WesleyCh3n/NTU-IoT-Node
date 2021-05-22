CC := arm-linux-gnueabihf-g++
CCFLAGS := -std=c++17 -O3
SRC := ./yolov4-tiny.cc
OBJFILES := ./lib/yolov4-tiny.o
LIBFILES := ./lib/libyolov4_tiny.a

NETWORK_DIR := ../network
OPENCV_DIR := /opt/opencv
TF_DIR := /opt/tensorflow_src

INCLUDES := \
-I$(OPENCV_DIR)/include/opencv4 \
-I$(TF_DIR)/tensorflow/lite/tools/make/downloads/flatbuffers/include \
-I$(TF_DIR) \
-I$(NETWORK_DIR) \
-I. \

LIBDIRS := \
-L$(TF_DIR)/tensorflow/lite/tools/make/gen/rpi_armv7l/lib \
-L$(OPENCV_DIR)/lib/ \
-L$(OPENCV_DIR)/lib/opencv4/3rdparty \
-L$(OPENCV_DIR)/build/lib \
-L$(NETWORK_DIR)/lib/ \

LIBS := \
-lstdc++fs \
-lnetwork \
-ltensorflow-lite \
`pkg-config --libs-only-l opencv4` \
-littnotify -llibprotobuf -llibwebp \
-llibopenjp2 -lIlmImf -lquirc \
-ltegra_hal -lade -ljpeg -lpng -ltiff \
-lz -lm -lpthread -ldl -lrt \
-Wl,-Bstatic -lssl -lcrypto -Wl,-Bdynamic

all:$(LIBFILES)

$(LIBFILES): $(OBJFILES)
	@echo "ar $@"
	@ar -rcs $(LIBFILES) $(OBJFILES)

$(OBJFILES): $(SRC)
	mkdir -p lib
	@echo "CC -c $@"
	@$(CC) $(CCFLAGS) -c -o $(OBJFILES) $(SRC) $(INCLUDES) $(LIBDIRS) $(LIBS)

clean:
	rm -rf lib
