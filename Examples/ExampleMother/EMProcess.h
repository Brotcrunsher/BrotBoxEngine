#pragma once
#include "BBE/BrotBoxEngine.h"

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
		((int32_t), type)
	)
};

class SubsystemProcess
{
private:
	bbe::SerializableList<Process> processes = bbe::SerializableList<Process>("processes.dat", "ParanoiaConfig", bbe::Undoable::YES);
	bbe::List<bbe::String> foundGames;

public:
	void update();

	bool isGameOn() const;

	void drawGui();
};
