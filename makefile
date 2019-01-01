SRC_DIR := BrotBoxEngine
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(SRC_DIR)/obj/%.o,$(SRC_FILES))

SRC_DIR_EPG := ExampleParticleGravity
SRC_FILES_EPG := $(wildcard $(SRC_DIR_EPG)/*.cpp)
OBJ_FILES_EPG := $(patsubst $(SRC_DIR_EPG)/%.cpp,$(SRC_DIR_EPG)/obj/%.o,$(SRC_FILES_EPG))

all: ExampleParticleGravity

ExampleParticleGravity: EPG BBE

EPG: $(OBJ_FILES_EPG) BBE
	g++ -o ExampleParticleGravity/$@.exec $(OBJ_FILES_EPG) BBE.o -lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -pthread -lXi -ldl -ldmx -lXinerama -lXcursor -lvulkan

BBE: $(OBJ_FILES)
	#g++ -o $@ $^ -lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -pthread -lXi -ldl -ldmx -lXinerama -lXcursor -
	ld -r $^ -o $@.o

$(SRC_DIR)/obj/%.o: $(SRC_DIR)/%.cpp
	g++ -O3 -c -I ~/Schreibtisch/glfw-3.2.1/include -I Third-Party/stb -std=c++17 -o $@ $<

$(SRC_DIR_EPG)/obj/%.o: $(SRC_DIR_EPG)/%.cpp
	g++ -O3 -c -I ~/Schreibtisch/glfw-3.2.1/include -I Third-Party/stb -I BrotBoxEngine -std=c++17 -o $@ $<

clean:
	rm BrotBoxEngine/obj/*
	rm ExampleParticleGravity/obj/*
	rm ExampleParticleGravity/EPG
	rm BBE.o
