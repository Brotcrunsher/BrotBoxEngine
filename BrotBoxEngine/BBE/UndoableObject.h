#pragma once

#include "../BBE/list.h"


namespace bbe
{
	template<typename T>
	class UndoableObject
	{
	private:
		T current;
		bbe::List<T> history;
		int64_t historyIndex = 0;

	public:
		UndoableObject()
		{
			clearHistory();
		}
		explicit UndoableObject(const T& val) : current(val)
		{
			clearHistory();
		}

		void submit()
		{
			while (historyIndex + 1 < (int64_t)history.getLength())
			{
				history.popBack();
			}
			history.add(current);
			historyIndex++;
		}

		void clearHistory()
		{
			history.clear();
			history.add(current);
			historyIndex = 0;
		}

		void undo()
		{
			if (!isUndoable())
			{
				bbe::Crash(bbe::Error::IllegalState);
			}
			historyIndex--;
			current = history[historyIndex];
		}

		bool isUndoable() const
		{
			return historyIndex > 0;
		}

		void redo()
		{
			if (!isRedoable())
			{
				bbe::Crash(bbe::Error::IllegalState);
			}
			historyIndex++;
			current = history[historyIndex];
		}

		bool isRedoable() const
		{
			return historyIndex + 1 < (int64_t)history.getLength();
		}

		T& get()
		{
			return current;
		}

		const T& get() const
		{
			return current;
		}
	};
}
