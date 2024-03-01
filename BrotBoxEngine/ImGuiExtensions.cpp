#include "BBE/ImGuiExtensions.h"

static ::bbe::Game* activeGame = nullptr;

static int InputTextCallback(ImGuiInputTextCallbackData* data)
{
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
	{
		bbe::String* str = (bbe::String*)data->UserData;
		str->resizeCapacity(data->BufTextLen + 1);
		data->Buf = str->getRaw();
	}
	return 0;
}

bool ImGui::bbe::InputText(const char* label, ::bbe::String& s, ImGuiInputTextFlags flags)
{
	// Callbacks currently not supported.
	IM_ASSERT((flags & ImGuiInputTextFlags_CallbackCompletion) == 0);
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackHistory   ) == 0);
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackAlways    ) == 0);
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackCharFilter) == 0);
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize    ) == 0);
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackEdit      ) == 0);
    
	flags |= ImGuiInputTextFlags_CallbackResize;

	return ImGui::InputText(label, s.getRaw(), s.getCapacity(), flags, InputTextCallback, &s);
}

void ImGui::bbe::tooltip(const char* text)
{
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) && ImGui::BeginTooltip())
	{
		ImGui::Text(text);
		ImGui::EndTooltip();
	}
}

void ImGui::bbe::tooltip(const ::bbe::String& text)
{
	tooltip(text.getRaw());
}

bool ImGui::bbe::combo(const char* label, const::bbe::List<::bbe::String>& selections, int32_t& selection)
{
	bool retVal = false;

	if (ImGui::BeginCombo(label, selections[selection].getRaw()))
	{
		for (int32_t i = 0; i < selections.getLength(); i++)
		{
			if (ImGui::Selectable(selections[i].getRaw()))
			{
				selection = i;
				retVal = true;
			}
		}
		ImGui::EndCombo();
	}

	return retVal;
}

bool ImGui::bbe::clickableText(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int size = vsnprintf(nullptr, 0, fmt, args);
	static ::bbe::List<char> buffer; // Avoid allocations
	buffer.resizeCapacity(size + 1);
	vsnprintf(buffer.getRaw(), size + 1, fmt, args);
	va_end(args);

	bool dummy = false;
	return ImGui::Selectable(buffer.getRaw(), &dummy);
}

bool ImGui::bbe::securityButton(const char* text, SecurityButtonFlags flags)
{
	const bool shiftPressed = activeGame->isKeyDown(::bbe::Key::LEFT_SHIFT);
	if (flags & SecurityButtonFlags_DontShowWOSecurityButton)
	{
		if (!shiftPressed)
		{
			ImGui::Text(""); // This is here to make sure ImGui::SameLine() behaves as expected.
			return false;
		}
	}
	const bool retVal = ImGui::Button(shiftPressed ? text : "[Shift]") && shiftPressed;
	ImGui::bbe::tooltip("Hold shift to activate this button.");
	return retVal;
}

bool ImGui::bbe::datePicker(const char* label, ::bbe::TimePoint* time)
{
	bool changed = false;
	static int orgYear = 0;
	static int orgMonth = 0;
	static int orgDay = 0;

	static int startColumn = 0;
	static int year = 0;
	static int month = 0;
	static int day = 0;
	static int daysInMonth = 0;
	if (ImGui::BeginPopup(label))
	{
		static constexpr int columnWidth = 30;
		static constexpr ImVec2 bSize(columnWidth, 0.0f);

		bool dataDirty = false;

		if (ImGui::Button("<<", bSize)) { dataDirty = true; year--; }
		ImGui::SameLine();
		if (ImGui::Button("<", bSize)) { dataDirty = true; month--; }
		ImGui::SameLine();
		ImGui::Text((::bbe::String(year) + "/" + month).getRaw());
		ImGui::SameLine(6.5f * columnWidth); // TODO: Wtf? Why 6.5?!
		if (ImGui::Button(">", bSize)) { dataDirty = true; month++; }
		ImGui::SameLine();
		if (ImGui::Button(">>", bSize)) { dataDirty = true; year++; }

		if (dataDirty)
		{
			if (month == 0) {
				month = 12;
				year--;
			}
			if (month == 13)
			{
				month = 1;
				year++;
			}
			// Technically we could go back until 1970, January. But this simplifies stuff. And really - who cares?
			if (year < 1971) year = 1971;
			startColumn = (int)::bbe::TimePoint::getFirstWeekdayOfMonth(year, (::bbe::Month)month);
			daysInMonth = ::bbe::TimePoint::getDaysInMonth(year, (::bbe::Month)month);
		}

		if (ImGui::BeginTable("table", 7, ImGuiTableFlags_RowBg))
		{
			ImGui::TableSetupColumn("1", ImGuiTableColumnFlags_WidthFixed, columnWidth);
			ImGui::TableSetupColumn("2", ImGuiTableColumnFlags_WidthFixed, columnWidth);
			ImGui::TableSetupColumn("3", ImGuiTableColumnFlags_WidthFixed, columnWidth);
			ImGui::TableSetupColumn("4", ImGuiTableColumnFlags_WidthFixed, columnWidth);
			ImGui::TableSetupColumn("5", ImGuiTableColumnFlags_WidthFixed, columnWidth);
			ImGui::TableSetupColumn("6", ImGuiTableColumnFlags_WidthFixed, columnWidth);
			ImGui::TableSetupColumn("7", ImGuiTableColumnFlags_WidthFixed, columnWidth);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::Text("Mo");
			ImGui::TableSetColumnIndex(1); ImGui::Text("Tu");
			ImGui::TableSetColumnIndex(2); ImGui::Text("We");
			ImGui::TableSetColumnIndex(3); ImGui::Text("Th");
			ImGui::TableSetColumnIndex(4); ImGui::Text("Fr");
			ImGui::TableSetColumnIndex(5); ImGui::Text("Sa");
			ImGui::TableSetColumnIndex(6); ImGui::Text("So");

			ImGui::TableNextRow();
			int column = startColumn;
			for (int i = 1; i <= daysInMonth; i++)
			{
				ImGui::TableSetColumnIndex(column);
				::bbe::String s(i);
				const bool isSelected = orgYear == year && orgMonth == month && orgDay == i;

				if (isSelected)
				{
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(7.0f, 0.6f, 0.6f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(7.0f, 0.9f, 0.9f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(7.0f, 0.8f, 0.8f));
				}
				if (ImGui::Button(s.getRaw(), bSize))
				{
					*time = ::bbe::TimePoint::fromDate(year, month, i).nextMorning();
					ImGui::CloseCurrentPopup();
					changed = true;
				}
				if (isSelected)
				{
					ImGui::PopStyleColor(3);
				}
				column++;
				if (column == 7)
				{
					column = 0;
					ImGui::TableNextRow();
				}
			}
			ImGui::EndTable();
		}
		ImGui::EndPopup();
	}
	if (ImGui::Button(time->toString().getRaw()))
	{
		year = time->getYear();
		month = (int)time->getMonth();
		day = time->getDay();
		startColumn = (int)::bbe::TimePoint::getFirstWeekdayOfMonth(year, (::bbe::Month)month);
		daysInMonth = ::bbe::TimePoint::getDaysInMonth(year, (::bbe::Month)month);

		orgYear = year;
		orgMonth = month;
		orgDay = day;

		ImGui::OpenPopup(label);
	}
	return changed;
}

void ImGui::bbe::INTERNAL::setActiveGame(::bbe::Game* game)
{
	activeGame = game;
}

bool ImGui::Button(const::bbe::String& s, const ImVec2& size)
{
	return ImGui::Button(s.getRaw(), size);
}

void ImGui::Text(const::bbe::String& s)
{
	ImGui::Text(s.getRaw());
}
