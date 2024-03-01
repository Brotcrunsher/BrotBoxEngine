#pragma once
#include "BBE/BrotBoxEngine.h"

struct Process
{
	bbe::String title;

	enum /*Non-Class*/ Type
	{
		TYPE_UNKNOWN = 0,
		TYPE_SYSTEM = 1,
		TYPE_OTHER = 2,
		TYPE_GAME = 3,
	};
	int32_t type = TYPE_UNKNOWN;


	// Non-Persisted Helper Data below.

	void serialize(bbe::ByteBuffer& buffer) const;
	static Process deserialize(bbe::ByteBufferSpan& buffer);
};

class SubsystemProcess
{
private:
	bbe::SerializableList<Process> processes = bbe::SerializableList<Process>("processes.dat", "ParanoiaConfig", bbe::Undoable::YES);
	bool m_isGameOn = false;

public:
	void update();

	bool isGameOn() const;

	void drawGui();
};
