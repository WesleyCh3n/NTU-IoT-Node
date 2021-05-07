CC := arm-linux-gnueabihf-g++
CCFLAGS := -Bstatic -std=c++17 -O3

SRC := ./main.cc
PROG := mqtt_status


MQTTC_DIR := /opt/paho.mqtt.c
MQTTCXX_DIR := /opt/paho.mqtt.cpp

INCLUDES := \
-I$(MQTTC_DIR)/build/install/include \
-I$(MQTTCXX_DIR)/build/install/include

LIBDIRS := \
-L$(MQTTC_DIR)/build/install/lib \
-L$(MQTTCXX_DIR)/build/install/lib

LIBS := \
-lpaho-mqttpp3 -lpaho-mqtt3as -lpaho-mqtt3cs \
-lz -lm -lpthread -ldl -lrt \
-Wl,-Bstatic -lssl -lcrypto -Wl,-Bdynamic

$(PROG):$(SRC)
	@echo "Compiling $(PROG)"
	@$(CC) $(CCFLAGS) -o $(PROG) $(SRC) $(INCLUDES) $(LIBDIRS) $(LIBS)
	@echo "Compiling Complete"
