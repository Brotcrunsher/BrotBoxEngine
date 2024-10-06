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
	bbe::List<bbe::String> m_foundTimewasters;
	bbe::List<bbe::String> getTimeWasterUrls() const;

public:
	void update();
	void drawGui();

	bool timeWasterFound() const;

	void enableHighConcentrationMode() const;
	void disableHighConcentrationMode() const;
};
