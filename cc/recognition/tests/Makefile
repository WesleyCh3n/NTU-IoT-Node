CC := arm-linux-gnueabihf-g++
CCFLAGS := -Bstatic -std=c++17 -O3

SRC := cam.cc
PROG := cam

OPENCV_DIR := /opt/opencv
RASPICAM_DIR := /opt/raspicam
VC_DIR := /opt/vc

INCLUDES := \
-I$(OPENCV_DIR)/include/opencv4 \
-I$(RASPICAM_DIR)/include \

LIBDIRS := \
-L$(OPENCV_DIR)/lib/ \
-L$(OPENCV_DIR)/lib/opencv4/3rdparty \
-L$(OPENCV_DIR)/build/lib \
-L$(RASPICAM_DIR)/lib \
-L$(VC_DIR)/lib/ \

LIBS := \
-lstdc++fs \
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
-lz -ldl -lm -lpthread -lrt \
-lraspicam -lraspicam_cv -lmmal -lmmal_core -lmmal_util


$(PROG):$(SRC)
	@echo "Compiling $(PROG)"
	@$(CC) $(CCFLAGS) -o $(PROG) $(SRC) $(INCLUDES) $(LIBDIRS) $(LIBS)
	@echo "Compiling Complete"
