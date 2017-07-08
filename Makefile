H_FILES := $(wildcard src/*.h)
CPP_FILES := $(wildcard src/*.cpp)
OUT_FILES := master slave
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
LD_FLAGS := -g 
CC_FLAGS := -std=c++11 -I src/ -g 

all: master slave

master: $(OBJ_FILES) src/cmd/master.cpp
	g++ $(LD_FLAGS) $(CC_FLAGS) -o $@ $^

slave: $(OBJ_FILES) src/cmd/slave.cpp
	g++ $(LD_FLAGS) $(CC_FLAGS) -o $@ $^

obj/%.o: src/%.cpp $(H_FILES)
	g++ $(CC_FLAGS) -c -o $@ $<

clean:
	rm -v $(OBJ_FILES) $(OUT_FILES)