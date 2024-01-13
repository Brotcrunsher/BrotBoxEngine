#pragma once

#include "../BBE/List.h"
#include "../BBE/String.h"
#include "../BBE/SimpleFile.h"
#include <ctime>

namespace bbe
{
	enum class Undoable
	{
		NO,
		YES,
	};

	template <typename T>
	class SerializableList
	{
	private:
		bbe::String path;
		bbe::String paranoiaPath;
		bbe::List<T> data;

		bbe::List<bbe::List<T>> history;
		Undoable undoable = Undoable::NO;

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

			pushUndoable();
		}

		void pushUndoable()
		{
			if (undoable == Undoable::YES)
			{
				history.add(data);
			}
		}

		SerializableList()
		{
			// Do nothing;
		}

	public:
		SerializableList(const bbe::String& path, const bbe::String& paranoiaPath = "", Undoable undoable = Undoable::NO) :
			paranoiaPath(paranoiaPath),
			undoable(undoable)
		{
			load(path, {});
		}

		static SerializableList withDefault(const bbe::String& path, const bbe::List<T>& data, const bbe::String& paranoiaPath = "", Undoable undoable = Undoable::NO)
		{
			SerializableList sl;
			sl.paranoiaPath = paranoiaPath;
			sl.undoable = undoable;
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
			pushUndoable();
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

		void writeToFile(bool updateHistory = true)
		{
			bbe::ByteBuffer buffer;
			for (size_t i = 0; i < data.getLength(); i++)
			{
				auto token = buffer.reserveSizeToken();
				data[i].serialize(buffer);
				buffer.fillSizeToken(token);
			}
			bbe::simpleFile::writeBinaryToFile(path, buffer);
			if (paranoiaPath.getLength() != 0)
			{
				time_t t;
				time(&t);
				bbe::simpleFile::createDirectory(paranoiaPath);
				bbe::simpleFile::writeBinaryToFile(paranoiaPath + "/" + path + t + ".bak", buffer);
			}
			
			if(updateHistory) pushUndoable();
		}

		bool canUndo() const
		{
			return history.getLength() > 1;
		}

		bool undo()
		{
			if (!canUndo()) return false;

			history.popBack();
			data = history.last();
			writeToFile(false);
			return true;
		}

		const bbe::List<T>& getList() const
		{
			return data;
		}
	};
}
