CC := arm-linux-gnueabihf-g++
CCFLAGS := -Bstatic -std=c++17 -O3

SRC := ./l2_dist.cc
PROG := main


BOOST_DIR := /opt/boost_1_75_0

INCLUDES := \
-I$(BOOST_DIR) \

LIBDIRS := \

LIBS := \

$(PROG):$(SRC)
	@echo "Compiling $(PROG)"
	@$(CC) $(CCFLAGS) -o $(PROG) $(SRC) $(INCLUDES) $(LIBDIRS) $(LIBS)
	@echo "Compiling Complete"
