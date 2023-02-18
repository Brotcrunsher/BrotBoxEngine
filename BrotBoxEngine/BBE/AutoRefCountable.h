#pragma once

#include <cstdint>

namespace bbe
{
	class AutoRef;
	class AutoRefCountable
	{
		friend class AutoRef;
	private:
		mutable int32_t refCount = 0;
	public:
		virtual ~AutoRefCountable();
	};

	class AutoRef
	{
	private:
		AutoRefCountable* countable = nullptr;

		void incRef();
		void decRef();

	public:
		AutoRef();
		/*none explicit*/ AutoRef(AutoRefCountable* countable);
		virtual ~AutoRef();

		AutoRef(const AutoRef& other);
		AutoRef(AutoRef&& other) noexcept;
		AutoRef& operator=(const AutoRef& other);
		AutoRef& operator=(AutoRef&& other) noexcept;
		AutoRef& operator=(AutoRefCountable* countable);

		bool operator==(const void* ptr) const;
		bool operator!=(const void* ptr) const;
		AutoRefCountable* get() const;
	};
}