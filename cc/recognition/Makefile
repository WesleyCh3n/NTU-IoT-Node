CC := arm-linux-gnueabihf-g++
CCFLAGS := -Bstatic -std=c++17 -O3
SRC := main.cc include/cow_monitor.cpp
TARGET := ntu-iot-node
OBJFILES := cow_monitor.o main.o

ifeq ($(RELEASE),1)
	CCFLAGS += -DRELEASE -DNTU_IOT_NODE_VERSION=\"$(VERSION)\"
endif

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
-I/home/pi/ntu-iot-node/cc/recognition

LIBDIRS := \
-L$(TF_DIR)/tensorflow/lite/tools/make/gen/rpi_armv7l/lib \
-L$(OPENCV_DIR)/lib/ \
-L$(OPENCV_DIR)/lib/opencv4/3rdparty \
-L$(OPENCV_DIR)/build/lib \
-L$(RASPICAM_DIR)/lib \
-L$(MQTTC_DIR)/build/install/lib \
-L$(MQTTCXX_DIR)/build/install/lib \
-L$(VC_DIR)/lib/

LIBS := \
-lstdc++fs \
-ltensorflow-lite \
-lopencv_gapi -lopencv_stitching -lopencv_aruco -lopencv_bgsegm \
-lopencv_bioinspired -lopencv_ccalib -lopencv_dnn_objdetect \
-lopencv_dnn_superres -lopencv_dpm -lopencv_face -lopencv_freetype \
-lopencv_fuzzy -lopencv_hfs -lopencv_img_hash -lopencv_intensity_transform \
-lopencv_line_descriptor -lopencv_mcc -lopencv_quality -lopencv_rapid \
-lopencv_reg -lopencv_rgbd -lopencv_saliency -lopencv_stereo \
-lopencv_structured_light -lopencv_phase_unwrapping -lopencv_superres \
-lopencv_optflow -lopencv_surface_matching -lopencv_tracking \
-lopencv_highgui -lopencv_datasets -lopencv_text -lopencv_plot \
-lopencv_videostab -lopencv_videoio -lopencv_xfeatures2d -lopencv_shape \
-lopencv_ml -lopencv_ximgproc -lopencv_video -lopencv_dnn \
-lopencv_xobjdetect -lopencv_objdetect -lopencv_calib3d -lopencv_imgcodecs \
-lopencv_features2d -lopencv_flann -lopencv_xphoto -lopencv_photo \
-lopencv_imgproc -lopencv_core -littnotify -llibprotobuf -llibwebp \
-llibopenjp2 -lIlmImf -lquirc -ltegra_hal -lade -ljpeg -lpng -ltiff \
-lpaho-mqttpp3 -lpaho-mqtt3as -lpaho-mqtt3cs \
-lz -lm -lpthread -ldl -lrt \
-lraspicam -lraspicam_cv -lmmal -lmmal_core -lmmal_util \
-Wl,-Bstatic -lssl -lcrypto -Wl,-Bdynamic

# make clean;make RELEASE=1 VERSION=2.0.1
all:$(TARGET)

$(TARGET):$(OBJFILES)
	@echo "Compiling $(TARGET)"
	@$(CC) $(CCFLAGS) -o $(TARGET) $(OBJFILES) $(INCLUDES) $(LIBDIRS) $(LIBS)
	@echo "Compiling Complete"

$(OBJFILES):$(COWMONITOR_SRC)
	@echo "Compiling VERSION: $(VERSION)"
	@echo "Compiling $(OBJFILES)"
	@$(CC) $(CCFLAGS) -c $(SRC) $(INCLUDES) $(LIBDIRS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(TARGET) cow_monitor.o main.o
