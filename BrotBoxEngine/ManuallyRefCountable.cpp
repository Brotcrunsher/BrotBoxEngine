#include "BBE/ManuallyRefCountable.h"

bbe::ManuallyRefCountable::~ManuallyRefCountable()
{
	 // Do nothing.
}

void bbe::ManuallyRefCountable::incRef()
{
	refCount++;
}

void bbe::ManuallyRefCountable::decRef()
{
	refCount--;
	if (refCount <= 0)
	{
		delete this;
	}
}
