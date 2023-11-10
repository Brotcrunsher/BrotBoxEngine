#pragma once

#include "../BBE/List.h"
#include "../BBE/String.h"
#include "../BBE/SimpleFile.h"
#include <ctime>

namespace bbe
{
	template <typename T>
	class SerializableList
	{
	public:
		enum class ParanoiaMode
		{
			NORMAL,
			PARANOIA
		};
	private:
		bbe::String path;
		bbe::List<T> data;
		ParanoiaMode paranoia;

		void load(const bbe::String& path, const bbe::List<T>& data)
		{
			this->path = path;
			if (bbe::simpleFile::doesFileExist(path))
			{
				// TODO: Check if load is even required
				bbe::ByteBuffer binary = bbe::simpleFile::readBinaryFile(path);
				bbe::ByteBufferSpan span = binary.getSpan();
				while (span.hasMore())
				{
					int64_t size;
					span.read(size);
					this->data.add(T::deserialize(span.readSpan(size)));
				}
			}
			else
			{
				this->data = data;
				writeToFile();
			}
		}

		SerializableList()
		{
			// Do nothing;
		}

	public:
		SerializableList(const bbe::String& path, ParanoiaMode paranoia = ParanoiaMode::NORMAL) :
			paranoia(paranoia)
		{
			load(path, {});
		}

		static SerializableList withDefault(const bbe::String& path, const bbe::List<T>& data, ParanoiaMode paranoia = ParanoiaMode::NORMAL)
		{
			SerializableList sl;
			sl.paranoia = paranoia;
			sl.load(path, data);
			return sl;
		}

		void add(const T& t)
		{
			data.add(t);
			bbe::ByteBuffer buffer;
			auto token = buffer.reserveSizeToken();
			t.serialize(buffer);
			buffer.fillSizeToken(token);
			bbe::simpleFile::appendBinaryToFile(path, buffer);
		}

		bool removeIndex(size_t index)
		{
			if (data.removeIndex(index))
			{
				writeToFile();
				return true;
			}
			return false;
		}

		bool swap(size_t a, size_t b)
		{
			bool retVal = data.swap(a, b);
			if(retVal) writeToFile();
			return retVal;
		}

		size_t getLength() const
		{
			return data.getLength();
		}

		T& operator[](size_t index)
		{
			return data[index];
		}

		const T& operator[](size_t index) const
		{
			return data[index];
		}

		void writeToFile()
		{
			bbe::ByteBuffer buffer;
			for (size_t i = 0; i < data.getLength(); i++)
			{
				auto token = buffer.reserveSizeToken();
				data[i].serialize(buffer);
				buffer.fillSizeToken(token);
			}
			bbe::simpleFile::writeBinaryToFile(path, buffer);
			if (paranoia == ParanoiaMode::PARANOIA)
			{
				time_t t;
				time(&t);
				bbe::simpleFile::writeBinaryToFile(path + t + ".bak", buffer);
			}
		}
	};
}
