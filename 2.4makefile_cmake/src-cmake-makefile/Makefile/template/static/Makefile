#target you can change test to what you want

#共享库文件名，lib*.a

TARGET := libtest.a
#compile and lib parameter

#编译参数
CC := gcc
AR = ar
RANLIB = ranlib
LIBS :=
LDFLAGS :=
DEFINES :=
INCLUDE := -I.

CFLAGS := -g -Wall -O3 $(DEFINES) $(INCLUDE)
CXXFLAGS:= $(CFLAGS) -DHAVE_CONFIG_H

#i think you should do anything here

#下面的基本上不需要做任何改动了

#source file

#源文件，自动找所有.c和.cpp文件，并将目标定义为同名.o文件

SOURCE := $(wildcard *.c) $(wildcard *.cpp)

OBJS := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))

.PHONY : everything objs clean veryclean rebuild

everything : $(TARGET)

all : $(TARGET)

objs : $(OBJS)

rebuild: veryclean everything

clean :
	rm -fr *.o

veryclean : clean
	rm -fr $(TARGET)

$(TARGET) : $(OBJS)
	$(AR) cru $(TARGET) $(OBJS)
	$(RANLIB) $(TARGET)
