objs := $(wildcard bsp/*.cpp)
objs += $(wildcard user/*.cpp)
objs += $(wildcard car_mode/*.cpp)
objs += $(wildcard mqtt/*.cpp)


SRC_PATH 	:= .

LINC 	+= -I$(PWD)/include/
LINC 	+= -I$(PWD)/include/mqtt/mqtt_packet/
LINC 	+= -I$(PWD)/include/mqtt/cJSON/
LINC 	+= -I$(PWD)/include/mqtt/
LINC 	+= -I$(PWD)/include/mqtt/ZHS/
LINC 	+= -I$(PWD)/include/mqtt/SQLITE3/

LINC 	+= -I$(PWD)/include/navigation/
LINC 	+= -I$(PWD)/user/

LIB     += -L$(PWD)/lib/mqtt_lib/
LIB     += -L$(PWD)/lib/hps3d_lib/

test : $(objs)
	#g++ -I include $(LINC) -o  $@ $^ -lpthread $(SRC_PATH)/mqtt_lib/libmqtta.a -D__USEDEBGU=0  -lm -ldl

	g++ -I include $(LINC) -o  $@ $^ -lpthread $(LIB) -l mqtta -l hps3d64 -Wl,-rpath=$(PWD)/lib/hps3d_lib/ -D__USEDEBGU=0  -lm -ldl -g
	
clean:
	rm -f test *.o 