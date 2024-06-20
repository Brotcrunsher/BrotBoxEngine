import os

name = input("Name: ")
if name == "":
    print("ERROR: Name must not be empty")
    exit(1)
name = name[0].upper() + name[1:]
if not name.startswith("Example"):
    name = "Example" + name
if os.path.isdir(name):
    print("ERROR: Project already exists.")
    exit(1)
print(name)
experimental = input("Experimental (y/n): ").lower()
if experimental not in ["y","n"]:
    print("ERROR: Experimental must be exactly y or n")
    exit(1)
experimental = True if experimental == "y" else False


os.makedirs(name)
with open(name + "/CMakeLists.txt", "w") as f:
    f.write("add_trivial_project(" + name + ")")


code = """#include "BBE/BrotBoxEngine.h"
#include <iostream>

class MyGame : public bbe::Game
{
	virtual void onStart() override
	{
	}
	virtual void update(float timeSinceLastFrame) override
	{
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(1280, 720, "Template!");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}""".replace("Template!", name)

with open(name + "/" + name + ".cpp", "w") as f:
    f.write(code)

with open("CMakeLists.txt", "a") as f:
    if experimental:
        f.write("if(BBE_ADD_EXPERIMENTAL)\n")
        f.write("  add_subdirectory(" + name + ")\n")
        f.write("endif()\n")
    else:
        f.write("add_subdirectory(" + name + ")\n")
