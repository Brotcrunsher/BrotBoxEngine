#pragma once

#include "BBE/BrotBoxEngine.h"

struct Url
{
	bbe::String url;
	enum /*Non-Class*/ Type
	{
		TYPE_UNKNOWN = 0,
		TYPE_TIME_WASTER = 1,
		TYPE_WORK = 2,
	};
	int32_t type = TYPE_UNKNOWN;


	// Non-Persisted Helper Data below.

	void serialDescription(bbe::SerializedDescription& desc) const;
};

class SubsystemUrl
{
private:
	bbe::SerializableList<Url> urls = bbe::SerializableList<Url>("urls.dat", "ParanoiaConfig", bbe::Undoable::YES);
	bbe::List<bbe::String> getDomains();
	bool m_timeWasterUrlFound = false;

public:
	void update();
	void drawGui();

	bool timeWasterFound() const;
};
