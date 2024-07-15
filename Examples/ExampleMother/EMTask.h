#pragma once

#include "BBE/BrotBoxEngine.h"

struct Task
{
	enum /*Non-Class*/ InputType
	{
		IT_NONE,
		IT_INTEGER,
		IT_FLOAT,
	};
	enum /*Non-Class*/ DateType
	{
		DT_DYNAMIC = 0,
		DT_YEARLY = 1,
		// dt_monthly = 2, // Not implemented
	};

	BBE_SERIALIZABLE_DATA(
		((bbe::String), title),
		((int32_t), repeatDays, 0),
		((bbe::TimePoint), previousExecution, bbe::TimePoint::epoch()),
		((bbe::TimePoint), nextExecution), // Call nextPossibleExecution from the outside! 
		((bool), canBeSu, true),
		((int32_t), followUp, 0), // In minutes. When clicking follow up, the task will be rescheduled the same day.
		((int32_t), internalValue, 0),
		((int32_t), internalValueIncrease, 0),
		((int32_t), followUp2, 0),
		((int32_t), inputType, (int32_t)IT_NONE),
		((bbe::List<float>), history),
		((bool), advanceable, false),
		((bool), oneShot, false),
		((bool), preparation, false),
		((bool), canBeMo, true),
		((bool), canBeTu, true),
		((bool), canBeWe, true),
		((bool), canBeTh, true),
		((bool), canBeFr, true),
		((bool), canBeSa, true),
		((bool), earlyAdvanceable, true),
		((bbe::String), clipboard),
		((bool), lateTimeTask, false),
		((int32_t), dateType, (int32_t)DT_DYNAMIC),
		((int32_t), dtYearlyMonth, 1),
		((int32_t), dtYearlyDay, 1),
		((bool), startable, false),
		((bbe::TimePoint), endWorkTime, bbe::TimePoint::epoch()),
		((bool), indefinitelyAdvanceable, false),
		((bool), shouldPlayNotificationSounds, true),
		((bbe::String), serverId),
		((bbe::TimePoint), overwriteTime)
	)

	// Non-Persisted Helper Data below.
	int32_t inputInt = 0;
	float inputFloat = 0;
	mutable bool armedToPlaySoundNewTask = false;
	mutable bool armedToPlaySoundDone = false;
	bbe::TimePoint execPointBuffer;
	bbe::TimePoint yearlyBuffer;

private:
	bool timePointElapsed(const bbe::TimePoint& tp, bool& armed) const;

public:
	bool shouldPlaySoundNewTask() const;
	bool shouldPlaySoundDone() const;

	void execDone();
	void execFollowUp();
	void execFollowUp2();
	void execMoveToNow();
	void execAdvance();

	void sanity();
	void nextExecPlusDays(int32_t days);
	bbe::TimePoint nextPossibleExecution() const;
	bool isPossibleWeekday(const bbe::TimePoint& tp) const;
	bbe::TimePoint toPossibleTimePoint(const bbe::TimePoint& tp, bool forwardInTime = true) const;
	bool isImportantTomorrow() const;
	bool isImportantToday() const;
	void setNextExecution(int32_t year, int32_t month, int32_t day);
	void setNextExecution(const bbe::TimePoint& tp);
	int32_t amountPossibleWeekdays() const;
	bbe::TimePoint getNextYearlyExecution() const;
	bool wasDoneToday() const;
	bbe::Duration getWorkDurationLeft() const;
	bool wasStartedToday() const;
	void execStart();
};

class SubsystemTask
{
public:
	bool forcePrepare = false;

private:
	bbe::SerializableList<Task> tasks = bbe::SerializableList<Task>("config.dat", "ParanoiaConfig", bbe::Undoable::YES);

	int32_t drawTable(const char* title, const std::function<bool(Task&)>& predicate, bool& requiresWrite,
		bool showMoveToNow,      bool showCountdown,  bool showDone,                bool showFollowUp,
		bool highlightRareTasks, bool showAdvancable, bool respectIndefinitelyFlag, bool sorted);

	bool isLateAdvanceableTime();


	bool isWorkTime() const;

public:

	void update();

	bool drawEditableTask(Task& t);
	bbe::Vector2 drawTabViewTasks();
	bbe::Vector2 drawTabEditTasks();

	void drawUndoButton();

	bool hasCurrentTask() const;
	bool hasPotentialTaskComplaint() const;
	bool isStreakFulfilled() const;

	void addServerTask(const bbe::String& id, const bbe::String& task);
};
