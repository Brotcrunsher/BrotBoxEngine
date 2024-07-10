#include "BBE/AutoRefCountable.h"
#include "BBE/Error.h"

bbe::AutoRefCountable::~AutoRefCountable()
{
	// Do nothing.
}

void bbe::AutoRef::incRef()
{
	if (countable)
	{
		countable->refCount++;
	}
}

void bbe::AutoRef::decRef()
{
	if (countable)
	{
		countable->refCount--;
		if (countable->refCount <= 0)
		{
			delete countable;
			countable = nullptr;
		}
	}
}

bbe::AutoRef::AutoRef() :
	countable(nullptr)
{
}

bbe::AutoRef::AutoRef(AutoRefCountable* countable) :
	countable(countable)
{
	if(countable) countable->refCount++;
}

bbe::AutoRef::~AutoRef()
{
	decRef();
}

bbe::AutoRef::AutoRef(const AutoRef& other)
{
	countable = other.countable;
	incRef();
}

bbe::AutoRef::AutoRef(AutoRef&& other) noexcept
{
	countable = other.countable;
	other.countable = nullptr;
}

bbe::AutoRef& bbe::AutoRef::operator=(const AutoRef& other)
{
	if (countable == other.countable) return *this;
	decRef();
	countable = other.countable;
	incRef();
	return *this;
}

bbe::AutoRef& bbe::AutoRef::operator=(AutoRef&& other) noexcept
{
	if (countable == other.countable)
	{
		other.countable = nullptr;
		decRef();
	}
	else
	{
		decRef();
		countable = other.countable;
		other.countable = nullptr;
	}
	return *this;
}

bbe::AutoRef& bbe::AutoRef::operator=(AutoRefCountable* countable)
{
	if (countable == this->countable) return *this;

	decRef();
	this->countable = countable;
	incRef();
	return *this;
}

bool bbe::AutoRef::operator==(const void* ptr) const
{
	return countable == ptr;
}

bool bbe::AutoRef::operator!=(const void* ptr) const
{
	return countable != ptr;
}

bbe::AutoRefCountable* bbe::AutoRef::get() const
{
	return countable;
}
