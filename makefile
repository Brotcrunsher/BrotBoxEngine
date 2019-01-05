GLFW_PATH := ~/Schreibtisch/glfw-3.2.1/include
COMPILER_FLAGS := -g -fsanitize=address -rdynamic

SRC_DIR := BrotBoxEngine
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(SRC_DIR)/obj/%.o,$(SRC_FILES))

SRC_DIR_EPG := ExampleParticleGravity
SRC_FILES_EPG := $(wildcard $(SRC_DIR_EPG)/*.cpp)
OBJ_FILES_EPG := $(patsubst $(SRC_DIR_EPG)/%.cpp,$(SRC_DIR_EPG)/obj/%.o,$(SRC_FILES_EPG))

SRC_DIR_TEST := BrotBoxEngineTest
SRC_FILES_TEST := $(wildcard $(SRC_DIR_TEST)/*.cpp)
OBJ_FILES_TEST := $(patsubst $(SRC_DIR_TEST)/%.cpp,$(SRC_DIR_TEST)/obj/%.o,$(SRC_FILES_TEST))


.PHONY: all clean dirs

all: ExampleParticleGravity

ExampleParticleGravity: EPG.exec Test.exec BBE.o



Test.exec: $(OBJ_FILES_TEST) BBE.o
	g++ -fsanitize=address -rdynamic -o $(SRC_DIR_TEST)/$@ $^ -lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -pthread -lXi -ldl -ldmx -lXinerama -lXcursor -lvulkan

EPG.exec: $(OBJ_FILES_EPG) BBE.o
	g++ -fsanitize=address -rdynamic -o $(SRC_DIR_EPG)/$@ $^ -lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -pthread -lXi -ldl -ldmx -lXinerama -lXcursor -lvulkan

BBE.o: $(OBJ_FILES)
	ld -r $^ -o $@



$(SRC_DIR_TEST)/obj/%.o: $(SRC_DIR_TEST)/%.cpp
	g++ $(COMPILER_FLAGS) -c -I $(GLFW_PATH) -I Third-Party/stb -I BrotBoxEngine -I BrotBoxEngine/BrotBoxEngineTest -I BrotBoxEngine/BrotBoxEngineTest/Tests -I BrotBoxEngine/BrotBoxEngineTest/Tests/DataStructures -std=c++17 -o $@ $<

$(SRC_DIR_EPG)/obj/%.o: $(SRC_DIR_EPG)/%.cpp
	g++ $(COMPILER_FLAGS) -c -I $(GLFW_PATH) -I Third-Party/stb -I BrotBoxEngine -std=c++17 -o $@ $<

$(SRC_DIR)/obj/%.o: $(SRC_DIR)/%.cpp
	g++ $(COMPILER_FLAGS) -c -I $(GLFW_PATH) -I Third-Party/stb -std=c++17 -o $@ $<



dirs:
	mkdir -p BrotBoxEngine/obj
	mkdir -p ExampleParticleGravity/obj
	mkdir -p BrotBoxEngineTest/obj

clean:
	rm -f BrotBoxEngine/obj/*
	rm -f ExampleParticleGravity/obj/*
	rm -f BrotBoxEngineTest/obj/*
	rm -f ExampleParticleGravity/EPG
	rm -f BBE.o
