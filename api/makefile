# file server makefile.

include ../mak/makefile.env
SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
TARGET = testrsyncoct

all:$(TARGET)

$(TARGET):$(OBJS)
	$(CXX) $(LD_FLAGS)  -o $@ $^ $(LIB) 

%.o:%.cpp
	$(CXX) $(C_FLAGS) $(INC) -c  $<

clean:
	rm -fr $(OBJS) $(TARGET) 
