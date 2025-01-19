#include "EMTask.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "AssetStore.h"

bool Task::timePointElapsed(const bbe::TimePoint& tp, bool& armed) const
{
	if (tp.hasPassed())
	{
		if (armed)
		{
			armed = false;
			return true;
		}
	}
	else
	{
		armed = true;
	}
	return false;
}

bool Task::shouldPlaySoundNewTask() const
{
	return timePointElapsed(nextPossibleExecution(), armedToPlaySoundNewTask);
}

bool Task::shouldPlaySoundDone() const
{
	return timePointElapsed(endWorkTime, armedToPlaySoundDone);
}

void Task::execDone()
{
	internalValue += internalValueIncrease;
	previousExecution = bbe::TimePoint();
	if (dateType == DT_DYNAMIC)
	{
		nextExecution = toPossibleTimePoint(previousExecution.nextMorning().plusDays(repeatDays - 1));
	}
	else if (dateType == DT_YEARLY)
	{
		nextExecution = getNextYearlyExecution();
	}
	else if (dateType == DT_MONTHLY)
	{
		nextExecution = getNextMonthlyExecution();
	}
}

void Task::execFollowUp()
{
	previousExecution = bbe::TimePoint();
	nextExecution = previousExecution.plusMinutes(followUp);
}

void Task::execFollowUp2()
{
	previousExecution = bbe::TimePoint();
	nextExecution = previousExecution.plusMinutes(followUp2);
}

void Task::execMoveToNow()
{
	armedToPlaySoundNewTask = false;
	nextExecution = bbe::TimePoint();
	overwriteTime = bbe::TimePoint();
}

void Task::execAdvance()
{
	internalValue += internalValueIncrease;
	previousExecution = bbe::TimePoint();
	if (dateType == DT_DYNAMIC)
	{
		if (preparation)
		{
			nextExecution = toPossibleTimePoint(bbe::TimePoint().nextMorning().plusDays(repeatDays));
		}
		else
		{
			if (nextExecution.hasPassed())
			{
				nextExecution = bbe::TimePoint();
			}
			nextExecution = toPossibleTimePoint(nextExecution.plusDays(repeatDays)).toMorning();
		}
	}
	else if (dateType == DT_YEARLY)
	{
		nextExecution = getNextYearlyExecution();
	}
	else if (dateType == DT_MONTHLY)
	{
		nextExecution = getNextMonthlyExecution();
	}
}

void Task::execContingentStart()
{
	contingentCountingStart = bbe::TimePoint();
	contingentRunning = true;
}

void Task::addContingent(const bbe::TimePoint& stopTime)
{
	if(stopTime > contingentCountingStart) collectedContingentSeconds += (stopTime - contingentCountingStart).toSeconds();
	contingentCountingStart = bbe::TimePoint::epoch();
}

void Task::execContingentStop(const bbe::TimePoint& stopTime)
{
	addContingent(stopTime);
	contingentRunning = false;
}

void Task::sanity()
{
	if (repeatDays < 1) repeatDays = 1;
	if (followUp < 0) followUp = 0;
}

void Task::nextExecPlusDays(int32_t days)
{
	nextExecution = toPossibleTimePoint(nextPossibleExecution().plusDays(days).toMorning(), days > 0);
}

bbe::TimePoint Task::nextPossibleExecution() const
{
	if (contingentTask)
	{
		const int32_t daysToAdd = collectedContingentSeconds / (contingentSecondsPerDay > 0 ? contingentSecondsPerDay : 1);
		if (daysToAdd <= 0) return bbe::TimePoint();
		return bbe::TimePoint().plusDays(daysToAdd).toMorning();
	}
	if (overwriteTime.isToday()) return nextExecution;
	if (!nextExecution.hasPassed()) return nextExecution;
	return toPossibleTimePoint(bbe::TimePoint());
}

bool Task::isPossibleWeekday(const bbe::TimePoint& tp) const
{
	if (!canBeMo && tp.isMonday())    return false;
	if (!canBeTu && tp.isTuesday())   return false;
	if (!canBeWe && tp.isWednesday()) return false;
	if (!canBeTh && tp.isThursday())  return false;
	if (!canBeFr && tp.isFriday())    return false;
	if (!canBeSa && tp.isSaturday())  return false;
	if (!canBeSu && tp.isSunday())    return false;
	return true;
}

bbe::TimePoint Task::toPossibleTimePoint(const bbe::TimePoint& tp, bool forwardInTime) const
{
	bbe::TimePoint retVal = tp;
	for (int32_t i = 0; i < 14; i++)
	{
		if (!isPossibleWeekday(retVal))
		{
			if (forwardInTime) retVal = retVal.nextMorning();
			else               retVal = retVal.plusDays(-1);
		}
		else
		{
			break;
		}
	}
	if (preparation && retVal.isToday()) retVal = retVal.nextMorning();
	return retVal;
}

bool Task::isImportantTomorrow() const
{
	//TODO: "Interesting" calculation for "tomorrow"...
	bbe::TimePoint tomorrow = bbe::TimePoint().nextMorning().plusDays(1).plusHours(-6);
	if (!isPossibleWeekday(tomorrow)) return false;
	if (contingentTask)
	{
		if (nextPossibleExecution() < tomorrow) return true;
	}
	else
	{
		if (nextExecution < tomorrow) return true;
	}

	return false;
}

bool Task::isImportantToday() const
{
	auto tp = nextPossibleExecution();
	if (tp.hasPassed()) return true;
	if (tp.isToday()) return true;
	return false;
}

void Task::setNextExecution(int32_t year, int32_t month, int32_t day)
{
	nextExecution = toPossibleTimePoint(bbe::TimePoint::fromDate(year, month, day).nextMorning());
}

void Task::setNextExecution(const bbe::TimePoint& tp)
{
	setNextExecution(tp.getYear(), (int32_t)tp.getMonth(), tp.getDay());
}

int32_t Task::amountPossibleWeekdays() const
{
	int32_t retVal = 0;
	if (canBeMo) retVal++;
	if (canBeTu) retVal++;
	if (canBeWe) retVal++;
	if (canBeTh) retVal++;
	if (canBeFr) retVal++;
	if (canBeSa) retVal++;
	if (canBeSu) retVal++;

	return retVal;
}

bbe::TimePoint Task::getNextYearlyExecution() const
{
	bbe::TimePoint now;
	return toPossibleTimePoint(bbe::TimePoint::fromDate(now.getYear() + 1, dtYearlyMonth, dtYearlyDay).nextMorning());
}

bbe::TimePoint Task::getNextMonthlyExecution() const
{
	bbe::TimePoint now;
	auto month = now.getMonth();
	auto year = now.getYear();
	if (month == bbe::Month::DECEMBER)
	{
		month = bbe::Month::JANUARY;
		year++;
	}
	else
	{
		month = bbe::Month((int)month + 1);
	}

	return toPossibleTimePoint(bbe::TimePoint::fromDate(year, month, dtMonthlyDay).nextMorning());
}

bool Task::wasDoneToday() const
{
	return previousExecution.isToday();
}

bool Task::wasStartedToday() const
{
	return endWorkTime.isToday();
}

void Task::execStart()
{
	endWorkTime = bbe::TimePoint().plusSeconds(internalValue);
}

bool Task::isRareTask() const
{
	return repeatDays > 1 || !isPossibleWeekday(nextPossibleExecution().plusDays(-1));
}

bbe::Duration Task::getWorkDurationLeft() const
{
	return endWorkTime - bbe::TimePoint();
}

bool SubsystemTask::drawContingentButton(Task& t)
{
	if (t.contingentCountingStart == bbe::TimePoint::epoch())
	{
		if (ImGui::Button("Start"))
		{
			t.execContingentStart();
			return true;
		}
	}
	else
	{
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
		if (ImGui::Button("Stop"))
		{
			t.execContingentStop(bbe::TimePoint());
			ImGui::PopStyleColor(3);
			return true;
		}
		ImGui::PopStyleColor(3);
	}
	return false;
}

int32_t SubsystemTask::drawTable(float scale, const char* title, const std::function<bool(Task&)>& predicate, bool& requiresWrite, bool showMoveToNow, bool showCountdown, bool showDone, bool showFollowUp, bool highlightRareTasks, bool showAdvancable, bool respectIndefinitelyFlag, bool sorted)
{
	int32_t amountDrawn = 0;
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), title);
	if (ImGui::BeginTable("table2", 7, ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn("AAA", ImGuiTableColumnFlags_WidthFixed, 200 * scale);
		if (showCountdown)  ImGui::TableSetupColumn("BBB", ImGuiTableColumnFlags_WidthFixed, 175 * scale);
		ImGui::TableSetupColumn("DDD", ImGuiTableColumnFlags_WidthFixed, 50 * scale);
		if (showFollowUp)   ImGui::TableSetupColumn("EEE", ImGuiTableColumnFlags_WidthFixed, 50 * scale);
		if (showFollowUp)   ImGui::TableSetupColumn("FFF", ImGuiTableColumnFlags_WidthFixed, 50 * scale);
		if (showAdvancable) ImGui::TableSetupColumn("GGG", ImGuiTableColumnFlags_WidthFixed, 50 * scale);
		if (showMoveToNow)  ImGui::TableSetupColumn("HHH", ImGuiTableColumnFlags_WidthFixed, 87.5f * scale);
		static bbe::List<size_t> indices; // Avoid allocations
		indices.clear();
		for (size_t i = 0; i < tasks.getLength(); i++)
		{
			Task& t = tasks[i];
			if (!predicate(t)) continue;
			indices.add(i);
		}
		if (sorted)
		{
			indices.sort([&](const size_t& a, const size_t& b)
				{
					return tasks[a].nextPossibleExecution() < tasks[b].nextPossibleExecution();
				});
		}
		size_t deletedTasks = 0;
		for (size_t indexindex = 0; indexindex < indices.getLength(); indexindex++)
		{
			const size_t i = indices[indexindex] - deletedTasks;
			Task& t = tasks[i];
			if (!predicate(t)) continue;
			amountDrawn++;
			ImGui::PushID(i);
			ImGui::TableNextRow();
			if (ImGui::TableGetHoveredRow() == indexindex) ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, 0xFF333333);
			int32_t column = 0;
			ImGui::TableSetColumnIndex(column++);
			if (!t.serverId.isEmpty())
			{
				ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "(!)");
				ImGui::bbe::tooltip("This Task originated from the Server");
				ImGui::SameLine();
			}
			if ((highlightRareTasks && t.isRareTask()) || t.oneShot)
			{
				const bool poosibleTodoToday = (t.nextPossibleExecution().hasPassed() || t.nextPossibleExecution().isToday());
				if (t.oneShot) { ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "(!)"); ImGui::bbe::tooltip("A one shot task."); }
				else if (poosibleTodoToday) { ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.8f, 1.0f), "(?)"); ImGui::bbe::tooltip("A rare task that could be done today."); }
				else { ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "(!)"); ImGui::bbe::tooltip("A rare task."); }
				ImGui::SameLine();
			}
			bbe::String modifiedTitle = t.title;
			if (modifiedTitle.contains("[MIN]"))
			{
				auto value = t.internalValue / 60;
				modifiedTitle = modifiedTitle.replace("[MIN]", bbe::String(value));
			}
			if (modifiedTitle.contains("[SEC]"))
			{
				auto value = t.internalValue % 60;
				modifiedTitle = modifiedTitle.replace("[SEC]", bbe::String::format("%.2d", value));
			}
			if (t.clipboard[0] == '\0')
			{
				const bbe::String printableTitle = bbe::String::format(modifiedTitle.getRaw(), t.internalValue);
				if (t.lateTimeTask && isWorkTime())
				{
					ImGui::TextColored({ 0.3f, 0.3f, 0.3f, 1.0f }, printableTitle);
				}
				else
				{
					ImGui::Text(printableTitle);
				}
			}
			else
			{
				if (ImGui::bbe::clickableText(modifiedTitle.getRaw(), t.internalValue))
				{
					ImGui::SetClipboardText(t.clipboard.getRaw());
				}
			}
			ImGui::bbe::tooltip(modifiedTitle);
			if (t.history.getLength() > 1)
			{
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) && ImGui::BeginTooltip())
				{
					ImGui::PlotLines("##History", t.history.getRaw(), t.history.getLength());
					ImGui::EndTooltip();
				}
			}
			if (showCountdown)
			{
				ImGui::TableSetColumnIndex(column++);
				bbe::String s = (t.nextPossibleExecution() - bbe::TimePoint()).toString();
				const char* c = s.getRaw();
				ImGui::SetCursorPosX(
					+ImGui::GetCursorPosX()
					+ ImGui::GetColumnWidth()
					- ImGui::CalcTextSize(c).x
					- ImGui::GetScrollX()
					- 10 * ImGui::GetStyle().ItemSpacing.x);
				ImGui::Text(c);
				ImGui::bbe::tooltip(t.nextPossibleExecution().toString());
			}
			ImGui::TableSetColumnIndex(column++);
			if (showDone)
			{
				if (t.inputType == Task::IT_NONE)
				{
					bool showDoneButton = true;
					if (t.startable)
					{
						if (!t.wasDoneToday())
						{
							bbe::Duration dur = t.getWorkDurationLeft();
							if (!t.wasStartedToday())
							{
								showDoneButton = false;
								if (ImGui::Button("Start"))
								{
									t.execStart();
									requiresWrite = true;
								}
							}
							else if (!dur.isNegative())
							{
								ImGui::Text(dur.toString().getRaw());
								showDoneButton = false;
							}
						}
					}

					if (showDoneButton)
					{
						if (!t.oneShot)
						{
							if (!t.contingentTask)
							{
								if (ImGui::Button("Done"))
								{
									t.execDone();
									requiresWrite = true;
								}
							}
							else
							{
								requiresWrite |= drawContingentButton(t);
							}
						}
						else
						{
							if (ImGui::bbe::securityButton("Done"))
							{
								tasks.removeIndex(i);
								// Doesn't require write cause removeIndex already does that.
								deletedTasks++;
							}
						}
					}
				}
				else if (t.inputType == Task::IT_INTEGER)
				{
					if (ImGui::InputInt("##input", &t.inputInt, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue))
					{
						t.history.add(t.inputInt);
						t.execDone();
						requiresWrite = true;
					}
				}
				else if (t.inputType == Task::IT_FLOAT)
				{
					if (ImGui::InputFloat("##input", &t.inputFloat, 0, 0, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
					{
						t.history.add(t.inputFloat);
						t.execDone();
						t.historyViewDirty = true;
						requiresWrite = true;
					}
				}
				else
				{
					bbe::Crash(bbe::Error::IllegalState);
				}
			}
			if (showFollowUp)
			{
				ImGui::TableSetColumnIndex(column++);
				if (t.followUp > 0 && ImGui::Button((bbe::String("+") + t.followUp + "min").getRaw()))
				{
					t.execFollowUp();
					requiresWrite = true;
				}
				ImGui::TableSetColumnIndex(column++);
				if (t.followUp2 > 0 && ImGui::Button((bbe::String("+") + t.followUp2 + "min").getRaw()))
				{
					t.execFollowUp2();
					requiresWrite = true;
				}
			}
			if (showAdvancable)
			{
				ImGui::TableSetColumnIndex(column++);
				if (t.advanceable)
				{
					if (!t.contingentTask)
					{
						if (   (!respectIndefinitelyFlag || t.indefinitelyAdvanceable)
							&& (t.earlyAdvanceable || isLateAdvanceableTime() || forcePrepare)
							&& ImGui::Button("Advance"))
						{
							t.execAdvance();
							requiresWrite = true;
						}
					}
					else if(title != bbe::String("Later"))
					{
						requiresWrite |= drawContingentButton(t);
					}
				}
			}
			if (showMoveToNow && !t.contingentTask)
			{
				ImGui::TableSetColumnIndex(column++);
				if (ImGui::Button("Move to Now"))
				{
					t.execMoveToNow();
					requiresWrite = true;
				}
			}
			ImGui::PopID();
		}
		ImGui::EndTable();
	}
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();
	return amountDrawn;
}

bool SubsystemTask::isLateAdvanceableTime()
{
	return bbe::TimePoint() > bbe::TimePoint::todayAt(18, 00);
}

bool SubsystemTask::isWorkTime() const
{
	bbe::TimePoint now;
	return now > bbe::TimePoint::todayAt(5, 00) && now < bbe::TimePoint::todayAt(17, 00);
}

SubsystemTask::SubsystemTask()
{
	if (heartbeat->time > bbe::TimePoint::epoch())
	{
		bool requiresWrite = false;
		for (size_t i = 0; i < tasks.getLength(); i++)
		{
			if (tasks[i].contingentTask && tasks[i].contingentCountingStart != bbe::TimePoint::epoch())
			{
				tasks[i].execContingentStop(heartbeat->time);
				requiresWrite = true;
			}
		}
		if (requiresWrite)
		{
			tasks.writeToFile();
		}
	}
}

void SubsystemTask::update()
{
	bbe::List<Task>& taskList = tasks.getList();
	bool requiresWrite = false;
	if (taskList.any([](const Task& t) { return t.shouldPlaySoundNewTask(); }))
	{
		assetStore::NewTask()->play();
	}
	if (taskList.any([](const Task& t) { return t.shouldPlaySoundDone(); }))
	{
		assetStore::Done()->play();
	}
	for (Task& t : taskList)
	{
		if (t.contingentTask)
		{
			const bbe::TimePoint today = bbe::TimePoint().toMorning();
			if (today < bbe::TimePoint()) // Only update the contingent when we are after the current morning.
			{
				const int32_t days = (today - t.previousContingentSubtraction).toDays();
				if (days > 0)
				{
					t.previousContingentSubtraction = today;
					requiresWrite = true;
					t.collectedContingentSeconds -= days * t.contingentSecondsPerDay;
					if (t.collectedContingentSeconds < 0) t.collectedContingentSeconds = 0;
				}
			}

			if (t.stopContingentWhenLocked)
			{
				if (sessionLockMonitor.isScreenLocked()
					&& t.contingentCountingStart != bbe::TimePoint::epoch()
					&& t.contingentRunning)
				{
					t.addContingent(bbe::TimePoint());
					requiresWrite = true;
				}
				if (!sessionLockMonitor.isScreenLocked()
					&& t.contingentRunning
					&& t.contingentCountingStart == bbe::TimePoint::epoch())
				{
					t.execContingentStart();
					requiresWrite = true;
				}
			}
		}
	}
	if (requiresWrite)
	{
		tasks.writeToFile(false); // Updating history because of contingent update feels "weird".
	}

	EVERY_MINUTES(1)
	{
		heartbeat->time = bbe::TimePoint();
		heartbeat.writeToFile();
	}
}

static bool weekdayCheckbox(const char* label, bool* b, int32_t amountOfWeekdays)
{
	ImGui::BeginDisabled(amountOfWeekdays <= 1 && *b);
	const bool retVal = ImGui::Checkbox(label, b);
	ImGui::EndDisabled();
	return retVal;
}

bool SubsystemTask::drawEditableTask(Task& t)
{
	bool taskChanged = false;
	taskChanged |= ImGui::bbe::InputText("Title", t.title);
	taskChanged |= ImGui::bbe::combo("Date Type", { "Dynamic", "Yearly", "Monthly" }, &t.dateType);
	if (t.dateType == Task::DT_DYNAMIC)
	{
		taskChanged |= ImGui::InputInt("Repeat Days", &t.repeatDays);
	}
	else if(t.dateType == Task::DT_YEARLY)
	{
		ImGui::Text("Month/Day: ");
		ImGui::SameLine();
		ImGui::bbe::datePicker("Yearly Pick", &t.yearlyBuffer);
		// TODO: It's possible to change the year in the date picker, which is kinda dumb
		//       for a yearly task. The year is discarded, but the GUI could be nicer.

		t.dtYearlyMonth = (int32_t)t.yearlyBuffer.getMonth();
		t.dtYearlyDay = t.yearlyBuffer.getDay();
	}
	else if (t.dateType == Task::DT_MONTHLY)
	{
		ImGui::Text("Day: ");
		ImGui::SameLine();
		ImGui::InputInt("##MonthlyPick", &t.dtMonthlyDay);
		t.dtMonthlyDay = bbe::Math::clamp(t.dtMonthlyDay, 1, 28); // TODO: What about days after the 28th?
	}
	const int32_t amountOfWeekdays = t.amountPossibleWeekdays();
	taskChanged |= weekdayCheckbox("Monday", &t.canBeMo, amountOfWeekdays);
	ImGui::SameLine(); taskChanged |= weekdayCheckbox("Tuesday",   &t.canBeTu, amountOfWeekdays);
	ImGui::SameLine(); taskChanged |= weekdayCheckbox("Wednesday", &t.canBeWe, amountOfWeekdays);
	ImGui::SameLine(); taskChanged |= weekdayCheckbox("Thursday",  &t.canBeTh, amountOfWeekdays);
	ImGui::SameLine(); taskChanged |= weekdayCheckbox("Friday",    &t.canBeFr, amountOfWeekdays);
	ImGui::SameLine(); taskChanged |= weekdayCheckbox("Saturday",  &t.canBeSa, amountOfWeekdays);
	ImGui::SameLine(); taskChanged |= weekdayCheckbox("Sunday",    &t.canBeSu, amountOfWeekdays);
	
	taskChanged |= ImGui::Checkbox("Advanceable", &t.advanceable);
	ImGui::bbe::tooltip("Can \"done\" even if it's not planned for today.");
	if (t.advanceable)
	{
		ImGui::Indent(15.0f);
		taskChanged |= ImGui::Checkbox("Preparation", &t.preparation);
		ImGui::bbe::tooltip("Will never be shown for the current day. Inteded for Tasks that prepare stuff for tomorrow, e.g. pre brewing some coffee.");

		taskChanged |= ImGui::Checkbox("Early Advanceable", &t.earlyAdvanceable);
		ImGui::bbe::tooltip("If unchecked, the task is only advanceable after 18:00.");

		taskChanged |= ImGui::Checkbox("Indefinitely Advanceable", &t.indefinitelyAdvanceable);
		ImGui::bbe::tooltip("Can be advanced in the \"Later\" table.");

		if (ImGui::Checkbox("Contingent Task", &t.contingentTask))
		{
			taskChanged = true;
			t.contingentCountingStart = bbe::TimePoint::epoch();
			t.previousContingentSubtraction = bbe::TimePoint().toMorning();
			t.collectedContingentSeconds = 0;
		}
		ImGui::bbe::tooltip("A contingent task requires you to do X seconds of work every day. You can always start and stop the timer. When the timer is stopped, the spent time is added to the contingent.");
		if (t.contingentTask)
		{
			ImGui::Indent(15.0f);
			taskChanged |= ImGui::InputInt("Collected Contingent (Seconds)", &t.collectedContingentSeconds);
			taskChanged |= ImGui::InputInt("Contingent Per Day (Seconds)", &t.contingentSecondsPerDay);
			taskChanged |= ImGui::Checkbox("Stop Contingent Counting when Locked", &t.stopContingentWhenLocked);
			ImGui::Unindent(15.0f);
		}
		ImGui::Unindent(15.0f);
	}
	taskChanged |= ImGui::Checkbox("One Shot", &t.oneShot);
	ImGui::bbe::tooltip("Delets the Task when Done.");
	taskChanged |= ImGui::Checkbox("Late Time Task", &t.lateTimeTask);
	ImGui::bbe::tooltip("A late time task triggers the \"Open Tasks\" sound only outside of Working Hours.");
	taskChanged |= ImGui::Checkbox("Startable", &t.startable);
	ImGui::bbe::tooltip("Doesn't show \"Done\" immediately, but instead a start button that starts a count down of the length\nof the internal value in seconds. After that time a sound is played and the \"Done\" Button appears.");
	taskChanged |= ImGui::Checkbox("Play Notifications", &t.shouldPlayNotificationSounds);
	ImGui::bbe::tooltip("If set, playing a notification sound when time wasters are open and the task isn't done. Else, play no sound.");
	taskChanged |= ImGui::InputInt("Follow Up  (in Minutes)", &t.followUp);
	ImGui::bbe::tooltip("Pushes the Task by this many minutes into the future. Useful for Tasks that can be fulfilled multiple times per day.");
	taskChanged |= ImGui::InputInt("Follow Up2 (in Minutes)", &t.followUp2);
	ImGui::bbe::tooltip("Pushes the Task by this many minutes into the future. Useful for Tasks that can be fulfilled multiple times per day.");
	taskChanged |= ImGui::InputInt("Internal Value", &t.internalValue);
	ImGui::bbe::tooltip("An internal value that can be printed out in the title via %%d, [SEC], and [MIN].");
	taskChanged |= ImGui::InputInt("Internal Value Increase", &t.internalValueIncrease);
	ImGui::bbe::tooltip("Increases the Internal Value on ever Done by this much.");
	taskChanged |= ImGui::bbe::combo("Input Type", { "None", "Integer", "Float" }, &t.inputType);
	if (t.inputType == Task::IT_FLOAT)
	{
		taskChanged |= ImGui::InputInt("History Target Anchor", &t.historyTargetAnchor);
		ImGui::bbe::tooltip("The starting point (x-value) where to begin the target line.");
		taskChanged |= ImGui::InputFloat("History Target Start Value", &t.historyTargetStartValue);
		ImGui::bbe::tooltip("The starting point (y-value) where to begin the target line.");
		taskChanged |= ImGui::InputFloat("History Target Change Value", &t.historyTargetChangeValue);
		ImGui::bbe::tooltip("The change of the target line per x-step.");
	}

	taskChanged |= ImGui::bbe::InputText("Clipboard", t.clipboard);
	ImGui::bbe::tooltip("When clicking the task, this will be sent to your clipboard.");

	return taskChanged;
}

bbe::Vector2 SubsystemTask::drawTabViewTasks(float scale)
{
	{
		static Task temp;
		ImGui::Text("New One Shot:");
		ImGui::SameLine();
		if (ImGui::bbe::InputText("##bla", temp.title, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			temp.oneShot = true;
			tasks.add(temp);
			temp = Task();
			ImGui::SetKeyboardFocusHere(-1);
		}
	}

	bool requiresWrite = false;
	drawTable(scale, "Now", [](Task& t) { return t.nextPossibleExecution().hasPassed() && !t.preparation; }, requiresWrite, false, false, true, true, false, false, false, false);
	drawTable(scale, "Today", [](Task& t) { return !t.nextPossibleExecution().hasPassed() && t.nextPossibleExecution().isToday(); }, requiresWrite, true, true, true, true, false, false, false, false);
	drawTable(scale, "Tomorrow", [](Task& t) { return t.isImportantTomorrow(); }, requiresWrite, true, false, false, true, true, true, false, false);
	drawTable(scale, "Later", [](Task& t) { return !t.nextPossibleExecution().hasPassed() && !t.nextPossibleExecution().isToday() && !t.isImportantTomorrow(); }, requiresWrite, true, true, true, false, false, true, true, true);
	if (requiresWrite)
	{
		tasks.writeToFile();
	}
	return bbe::Vector2(1);
}

bbe::Vector2 SubsystemTask::drawTabEditTasks()
{
	{
		static Task tempTask;
		drawEditableTask(tempTask);
		tempTask.sanity();

		static bbe::TimePoint firstExec;

		ImGui::Text("First execution: ");
		ImGui::SameLine();
		ImGui::bbe::datePicker("First Exec", &firstExec);

		if (ImGui::Button("New Task"))
		{
			tempTask.setNextExecution(firstExec);
			tasks.add(tempTask);
			tempTask = Task();
		}
	}
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::Separator();
	ImGui::Separator();
	static char searchBuffer[128] = {};
	ImGui::InputText("Search", searchBuffer, sizeof(searchBuffer));
	ImGui::Separator();
	ImGui::Separator();
	ImGui::Separator();
	ImGui::NewLine();

	bool tasksChanged = false;
	size_t deletionIndex = (size_t)-1;
	for (size_t i = 0; i < tasks.getLength(); i++)
	{
		Task& t = tasks[i];
		if (searchBuffer[0] != 0 && !bbe::String(t.title).containsIgnoreCase(searchBuffer)) continue;
		ImGui::PushID(i);
		if (ImGui::bbe::securityButton("Delete Task"))
		{
			deletionIndex = i;
		}
		if (i != 0)
		{
			ImGui::SameLine();
			if (ImGui::Button("Up"))
			{
				tasks.swap(i, i - 1);
			}
		}
		if (i != tasks.getLength() - 1)
		{
			ImGui::SameLine();
			if (ImGui::Button("Down"))
			{
				tasks.swap(i, i + 1);
			}
		}
		tasksChanged |= drawEditableTask(t);
		tasksChanged |= ImGui::bbe::datePicker("previousExe", &t.previousExecution); ImGui::bbe::tooltip("Previous Execution");
		t.execPointBuffer = t.nextPossibleExecution();
		const bool execPointChanged = ImGui::bbe::datePicker("nextExe", &t.execPointBuffer); ImGui::bbe::tooltip("Next Execution");
		if (execPointChanged)
		{
			t.setNextExecution(t.execPointBuffer);
			tasksChanged = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Move to Now"))
		{
			t.execMoveToNow();
			tasksChanged = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("+1 Day"))
		{
			t.nextExecPlusDays(1);
			tasksChanged = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("-1 Day"))
		{
			t.nextExecPlusDays(-1);
			tasksChanged = true;
		}
		if (t.startable)
		{
			tasksChanged |= ImGui::bbe::datePicker("EndWork", &t.endWorkTime); ImGui::bbe::tooltip("End Work Time");
		}
		ImGui::NewLine();
		ImGui::Separator();
		ImGui::NewLine();
		ImGui::PopID();
		t.sanity();
	}
	tasks.removeIndex(deletionIndex);
	if (tasksChanged)
	{
		tasks.writeToFile();
	}
	return bbe::Vector2(1);
}

bbe::Vector2 SubsystemTask::drawTabHistoryView()
{
	bbe::List<bbe::String> historyTitles;
	bbe::List<size_t> historyIndices;
	for (size_t i = 0; i < tasks.getLength(); i++)
	{
		if (tasks[i].inputType == (int32_t)Task::IT_FLOAT)
		{
			historyIndices.add(i);
			historyTitles.add(tasks[i].title);
		}
	}

	if (historyTitles.getLength() > 0)
	{
		static int32_t selection = 0;
		ImGui::bbe::combo("History Selection", historyTitles, &selection);

		Task& task = tasks[historyIndices[selection]];
		const bbe::List<float>& history = task.history;
		bbe::List<float> time;
		for (size_t i = 0; i < history.getLength(); i++)
		{
			time.add((float)i);
		}

		if (task.historyViewDirty)
		{
			task.historyViewDirty = false;
			ImPlot::SetNextAxesToFit();
		}

		bool shouldDrawTargetDifference = false;
		float lastTargetValue = 0.0f;
		if (ImPlot::BeginPlot("History", { -1, 250 })) {
			ImPlot::SetupAxes("Time", "Value");
			ImPlot::PlotLine(task.title.getRaw(), time.getRaw(), history.getRaw(), time.getLength());
			if (task.historyTargetAnchor >= 0 && history.getLength() > 0)
			{
				shouldDrawTargetDifference = true;
				bbe::List<float> targetTime;
				bbe::List<float> targetValue;

				for (size_t i = task.historyTargetAnchor; i < history.getLength(); i++)
				{
					targetTime.add((float)i);
					lastTargetValue = task.historyTargetStartValue + task.historyTargetChangeValue * (i - task.historyTargetAnchor);
					targetValue.add(lastTargetValue);
				}

				ImPlot::PlotLine("Target Line", targetTime.getRaw(), targetValue.getRaw(), targetTime.getLength());
			}
			ImPlot::EndPlot();
		}
		if (shouldDrawTargetDifference)
		{
			const float targetDifference = lastTargetValue - history.last();
			ImGui::Text("Target Difference: %.2f", targetDifference);
		}
	}

	return bbe::Vector2(1);
}

void SubsystemTask::drawUndoRedoButtons()
{
	ImGui::BeginDisabled(!tasks.canUndo());
	if (ImGui::Button("Undo"))
	{
		tasks.undo();
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!tasks.canRedo());
	if (ImGui::Button("Redo"))
	{
		tasks.redo();
	}
	ImGui::EndDisabled();
}

bool SubsystemTask::hasCurrentTask() const
{
	for (size_t i = 0; i < tasks.getLength(); i++)
	{
		const Task& t = tasks[i];
		if (t.nextPossibleExecution().hasPassed())
		{
			return true;
		}
	}
	return false;
}

bool SubsystemTask::hasPotentialTaskComplaint() const
{
	for (size_t i = 0; i < tasks.getLength(); i++)
	{
		const Task& t = tasks[i];
		if (t.nextPossibleExecution().hasPassed())
		{
			if (!t.oneShot && t.shouldPlayNotificationSounds)
			{
				if (!isWorkTime()) return true;
				if (!t.lateTimeTask) return true;
			}
		}
	}
	return false;
}

bool SubsystemTask::isStreakFulfilled() const
{
	for (size_t i = 0; i < tasks.getLength(); i++)
	{
		const Task& t = tasks[i];
		if (t.nextPossibleExecution().isToday() && !t.oneShot)
		{
			return false;
		}
	}
	return true;
}

void SubsystemTask::addServerTask(const bbe::String& id, const bbe::String& task)
{
	Task t;
	t.serverId = id;
	t.title = task;
	t.oneShot = true;
	t.clipboard = task;
	tasks.add(t);
}

bbe::List<bbe::String> SubsystemTask::getWarnings() const
{
	bbe::List<bbe::String> retVal;

	for (size_t i = 0; i < tasks.getLength(); i++)
	{
		const Task& t = tasks[i];
		if (t.contingentTask && t.contingentCountingStart != bbe::TimePoint::epoch())
		{
			retVal.add("Contingent Task running: " + t.title);
		}
	}

	return retVal;
}

bbe::TimePoint SubsystemTask::getHeartbeat() const
{
	return heartbeat->time;
}
