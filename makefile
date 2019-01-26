GLFW_PATH := Third-Party/GLFW/linux/include
COMPILER_FLAGS := -g -fsanitize=address -rdynamic -Wall -pedantic-errors
LINKER_DEPS := -lglfw3 -lGL -lGLU -lX11 -lXxf86vm -lXrandr -pthread -lXi -ldl -ldmx -lXinerama -lXcursor -lvulkan

SRC_DIR := BrotBoxEngine
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(SRC_DIR)/obj/%.o,$(SRC_FILES))

SRC_DIR_EPG := ExampleParticleGravity
SRC_FILES_EPG := $(wildcard $(SRC_DIR_EPG)/*.cpp)
OBJ_FILES_EPG := $(patsubst $(SRC_DIR_EPG)/%.cpp,$(SRC_DIR_EPG)/obj/%.o,$(SRC_FILES_EPG))

SRC_DIR_TEST := BrotBoxEngineTest
SRC_FILES_TEST := $(wildcard $(SRC_DIR_TEST)/*.cpp)
OBJ_FILES_TEST := $(patsubst $(SRC_DIR_TEST)/%.cpp,$(SRC_DIR_TEST)/obj/%.o,$(SRC_FILES_TEST))

SRC_DIR_3D := Example3D
SRC_FILES_3D := $(wildcard $(SRC_DIR_3D)/*.cpp)
OBJ_FILES_3D := $(patsubst $(SRC_DIR_3D)/%.cpp,$(SRC_DIR_3D)/obj/%.o,$(SRC_FILES_3D))

SRC_DIR_SNAKE := ExampleSnake
SRC_FILES_SNAKE := $(wildcard $(SRC_DIR_SNAKE)/*.cpp)
OBJ_FILES_SNAKE := $(patsubst $(SRC_DIR_SNAKE)/%.cpp,$(SRC_DIR_SNAKE)/obj/%.o,$(SRC_FILES_SNAKE))

SRC_DIR_MANDELBROT := ExampleMandelbrot
SRC_FILES_MANDELBROT := $(wildcard $(SRC_DIR_MANDELBROT)/*.cpp)
OBJ_FILES_MANDELBROT := $(patsubst $(SRC_DIR_MANDELBROT)/%.cpp,$(SRC_DIR_MANDELBROT)/obj/%.o,$(SRC_FILES_MANDELBROT))



.PHONY: all clean dirs

all: ExampleParticleGravity

ExampleParticleGravity: EPG.exec Test.exec E3D.exec Snake.exec Mandelbrot.exec BBE.o



Test.exec: $(OBJ_FILES_TEST) BBE.o
	g++ -fsanitize=address -rdynamic -o $(SRC_DIR_TEST)/$@ $^ $(LINKER_DEPS)

EPG.exec: $(OBJ_FILES_EPG) BBE.o
	g++ -fsanitize=address -rdynamic -o $(SRC_DIR_EPG)/$@ $^ $(LINKER_DEPS)

E3D.exec: $(OBJ_FILES_3D) BBE.o
	g++ -fsanitize=address -rdynamic -o $(SRC_DIR_3D)/$@ $^ $(LINKER_DEPS)

Snake.exec: $(OBJ_FILES_SNAKE) BBE.o
	g++ -fsanitize=address -rdynamic -o $(SRC_DIR_SNAKE)/$@ $^ $(LINKER_DEPS)
	
Mandelbrot.exec: $(OBJ_FILES_MANDELBROT) BBE.o
	g++ -fsanitize=address -rdynamic -o $(SRC_DIR_MANDELBROT)/$@ $^ $(LINKER_DEPS)

BBE.o: $(OBJ_FILES)
	ld -r $^ -o $@



$(SRC_DIR_TEST)/obj/%.o: $(SRC_DIR_TEST)/%.cpp
	g++ $(COMPILER_FLAGS) -Wno-sign-compare -c -I $(GLFW_PATH) -I Third-Party/Vulkan/Include -I Third-Party/stb -I BrotBoxEngine -I BrotBoxEngine/BrotBoxEngineTest -I BrotBoxEngine/BrotBoxEngineTest/Tests -I BrotBoxEngine/BrotBoxEngineTest/Tests/DataStructures -std=c++17 -o $@ $<

$(SRC_DIR_EPG)/obj/%.o: $(SRC_DIR_EPG)/%.cpp
	g++ --version
	g++ $(COMPILER_FLAGS) -c -I $(GLFW_PATH) -I Third-Party/Vulkan/Include -I Third-Party/stb -I BrotBoxEngine -std=c++17 -o $@ $<

$(SRC_DIR_3D)/obj/%.o: $(SRC_DIR_3D)/%.cpp
	g++ $(COMPILER_FLAGS) -c -I $(GLFW_PATH) -I Third-Party/Vulkan/Include -I Third-Party/stb -I BrotBoxEngine -std=c++17 -o $@ $<

$(SRC_DIR_SNAKE)/obj/%.o: $(SRC_DIR_SNAKE)/%.cpp
	g++ $(COMPILER_FLAGS) -c -I $(GLFW_PATH) -I Third-Party/Vulkan/Include -I Third-Party/stb -I BrotBoxEngine -std=c++17 -o $@ $<

$(SRC_DIR)/obj/%.o: $(SRC_DIR)/%.cpp
	g++ $(COMPILER_FLAGS) -c -I $(GLFW_PATH) -I Third-Party/Vulkan/Include -I Third-Party/stb -std=c++17 -o $@ $<



dirs:
	mkdir -p BrotBoxEngine/obj
	mkdir -p ExampleParticleGravity/obj
	mkdir -p BrotBoxEngineTest/obj
	mkdir -p Example3D/obj
	mkdir -p ExampleSnake/obj
	mkdir -p ExampleMandelbrot/obj

clean:
	rm -f BrotBoxEngine/obj/*
	rm -f ExampleParticleGravity/obj/*
	rm -f BrotBoxEngineTest/obj/*
	rm -f Example3D/obj/*
	rm -f ExampleSnake/obj/*
	rm -f ExampleMandelbrot/obj/*
	rm -f ExampleParticleGravity/EPG.exec
	rm -f BrotBoxEngineTest/Test.exec
	rm -f Example3D/E3D.exec
	rm -f ExampleSnake/Snake.exec
	rm -f ExampleMandelbrot/Mandelbrot.exec
	rm -f BBE.o
