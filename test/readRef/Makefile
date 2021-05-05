CC := arm-linux-gnueabihf-g++
CCFLAGS := -Bstatic -std=c++17 -O3

SRC := ./main.cc
PROG := main


INCLUDES := \

LIBDIRS := \

LIBS := \

$(PROG):$(SRC)
	@echo "Compiling $(PROG)"
	@$(CC) $(CCFLAGS) -o $(PROG) $(SRC) $(INCLUDES) $(LIBDIRS) $(LIBS)
	@echo "Compiling Complete"
