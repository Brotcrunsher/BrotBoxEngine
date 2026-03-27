#pragma once
#if defined(_WIN32) || defined(__linux__)
#include "BBE/BrotBoxEngine.h" // NOLINT(misc-include-cleaner): examples/tests intentionally use the engine umbrella.

struct Process
{
	enum /*Non-Class*/ Type
	{
		TYPE_UNKNOWN = 0,
		TYPE_SYSTEM = 1,
		TYPE_OTHER = 2,
		TYPE_GAME = 3,
	};
	BBE_SERIALIZABLE_DATA(
		((bbe::String), title),
		((int32_t), type),
		((bbe::String), exePath))
};

class SubsystemProcess
{
private:
	bbe::SerializableList<Process> processes = bbe::SerializableList<Process>("processes.dat", "ParanoiaConfig", bbe::Undoable::YES);
	bbe::List<bbe::String> foundGames;
	int32_t motherPorcesses = 0;

public:
	void update();

	bool isGameOn() const;

	void drawGui(float scale);

	bbe::List<bbe::String> getWarnings() const;
};
#endif
