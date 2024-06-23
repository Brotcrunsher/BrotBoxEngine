#pragma once

#include "BBE/BrotBoxEngine.h"

struct Url
{
	enum /*Non-Class*/ Type
	{
		TYPE_UNKNOWN = 0,
		TYPE_TIME_WASTER = 1,
		TYPE_WORK = 2,
	};
	BBE_SERIALIZABLE_DATA(
		((bbe::String), url),
		((int32_t), type)
	)
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
