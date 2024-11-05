#include "BBE/ImGuiExtensions.h"
#include "BBE/Window.h"

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

bool ImGui::bbe::combo(const char* label, const::bbe::List<::bbe::String>& selections, int32_t* selection)
{
	bool retVal = false;

	if (ImGui::BeginCombo(label, selections[*selection].getRaw()))
	{
		for (int32_t i = 0; i < selections.getLength(); i++)
		{
			if (ImGui::Selectable(selections[i].getRaw()))
			{
				*selection = i;
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
	::bbe::String s = ::bbe::String::formatVa(fmt, args);
	va_end(args);
	
	bool dummy = false;
	return ImGui::Selectable(s.getRaw(), &dummy);
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

bool ImGui::bbe::timePicker(const char* label, int32_t* hour, int32_t* minute)
{
	const ::bbe::String comboLabel1 = ::bbe::String("##1") + label;
	const ::bbe::String comboLabel2 = ::bbe::String("##2") + label;

	::bbe::List<::bbe::String> hourSelection;
	for (int i = 0; i < 24; i++)
	{
		hourSelection.add(::bbe::String(i));
	}
	::bbe::List<::bbe::String> minuteSelection;
	for (int i = 0; i < 60; i++)
	{
		minuteSelection.add(::bbe::String(i));
	}

	ImGui::Text("%s:", label);
	ImGui::SameLine();
	bool retVal = false;
	ImGui::PushItemWidth(50 * activeGame->getWindow()->getScale());
	retVal |= combo(comboLabel1.getRaw(), hourSelection,   hour);
	ImGui::SameLine();
	retVal |= combo(comboLabel2.getRaw(), minuteSelection, minute);
	ImGui::PopItemWidth();

	return retVal;
}

static ImU32 col = 0;
void ImGui::bbe::SetColor(ImU32 c)
{
	col = c;
}

float ImGui::bbe::ToResolution(float val)
{
	return activeGame->getWindow()->getScale() * val;
}

void ImGui::bbe::AddLine(const ImVec2& p1, const ImVec2& p2, float thickness)
{
	ImGui::GetForegroundDrawList()->AddLine(p1, p2, col, thickness);
}

void ImGui::bbe::AddRect(const ImVec2& p_min, const ImVec2& p_max, float rounding, ImDrawFlags flags, float thickness)
{
	ImGui::GetForegroundDrawList()->AddRect(p_min, p_max, col, rounding, flags, thickness);
}

void ImGui::bbe::AddRectFilled(const ImVec2& p_min, const ImVec2& p_max, float rounding, ImDrawFlags flags)
{
	ImGui::GetForegroundDrawList()->AddRectFilled(p_min, p_max, col, rounding, flags);
}

void ImGui::bbe::AddQuad(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, float thickness)
{
	ImGui::GetForegroundDrawList()->AddQuad(p1, p2, p3, p4, col, thickness);
}

void ImGui::bbe::AddQuadFilled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4)
{
	ImGui::GetForegroundDrawList()->AddQuadFilled(p1, p2, p3, p4, col);
}

void ImGui::bbe::AddTriangle(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, float thickness)
{
	ImGui::GetForegroundDrawList()->AddTriangle(p1, p2, p3, col, thickness);
}

void ImGui::bbe::AddTriangleFilled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3)
{
	ImGui::GetForegroundDrawList()->AddTriangleFilled(p1, p2, p3, col);
}

void ImGui::bbe::AddCircle(const ImVec2& center, float radius, int num_segments, float thickness)
{
	ImGui::GetForegroundDrawList()->AddCircle(center, ToResolution(radius), col, num_segments, thickness);
}

void ImGui::bbe::AddCircleFilled(const ImVec2& center, float radius, int num_segments)
{
	ImGui::GetForegroundDrawList()->AddCircleFilled(center, ToResolution(radius), col, num_segments);
}

void ImGui::bbe::AddNgon(const ImVec2& center, float radius, int num_segments, float thickness)
{
	ImGui::GetForegroundDrawList()->AddNgon(center, ToResolution(radius), col, num_segments, thickness);
}

void ImGui::bbe::AddNgonFilled(const ImVec2& center, float radius, int num_segments)
{
	ImGui::GetForegroundDrawList()->AddNgonFilled(center, ToResolution(radius), col, num_segments);
}

void ImGui::bbe::AddEllipse(const ImVec2& center, float radius_x, float radius_y, float rot, int num_segments, float thickness)
{
	ImGui::GetForegroundDrawList()->AddEllipse(center, ToResolution(radius_x), ToResolution(radius_y), col, rot, num_segments, thickness);
}

void ImGui::bbe::AddEllipseFilled(const ImVec2& center, float radius_x, float radius_y, float rot, int num_segments)
{
	ImGui::GetForegroundDrawList()->AddEllipseFilled(center, ToResolution(radius_x), ToResolution(radius_y), col, rot, num_segments);
}

void ImGui::bbe::AddText(const ImVec2& pos, const char* text_begin, const char* text_end)
{
	ImGui::GetForegroundDrawList()->AddText(pos, col, text_begin, text_end);
}

void ImGui::bbe::AddText(const ImFont* font, float font_size, const ImVec2& pos, const char* text_begin, const char* text_end, float wrap_width, const ImVec4* cpu_fine_clip_rect)
{
	ImGui::GetForegroundDrawList()->AddText(font, font_size, pos, col, text_begin, text_end, wrap_width, cpu_fine_clip_rect);
}

void ImGui::bbe::AddPolyline(const ImVec2* points, int num_points, ImDrawFlags flags, float thickness)
{
	ImGui::GetForegroundDrawList()->AddPolyline(points, num_points, col, flags, thickness);
}

void ImGui::bbe::AddConvexPolyFilled(const ImVec2* points, int num_points)
{
	ImGui::GetForegroundDrawList()->AddConvexPolyFilled(points, num_points, col);
}

void ImGui::bbe::AddBezierCubic(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, float thickness, int num_segments)
{
	ImGui::GetForegroundDrawList()->AddBezierCubic(p1, p2, p3, p4, col, thickness, num_segments);
}

void ImGui::bbe::AddBezierQuadratic(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, float thickness, int num_segments)
{
	ImGui::GetForegroundDrawList()->AddBezierQuadratic(p1, p2, p3, col, thickness, num_segments);
}





ImVec2 ImGui::bbe::window::ToWindowCoord(const ImVec2& p)
{
	ImVec2 w = ImGui::GetWindowPos();
	return ImVec2(p.x + w.x, p.y + w.y);
}

::bbe::List<ImVec2> ImGui::bbe::window::ToWindowCoord(const ImVec2* points, int num_points)
{
	::bbe::List<ImVec2> retVal;
	for (int i = 0; i < num_points; i++)
	{
		retVal.add(ToWindowCoord(points[i]));
	}
	return retVal;
}

void ImGui::bbe::window::AddLine(const ImVec2& p1, const ImVec2& p2, float thickness)
{
	ImGui::GetForegroundDrawList()->AddLine(ToWindowCoord(p1), ToWindowCoord(p2), col, thickness);
}

void ImGui::bbe::window::AddRect(const ImVec2& p_min, const ImVec2& p_max, float rounding, ImDrawFlags flags, float thickness)
{
	ImGui::GetForegroundDrawList()->AddRect(ToWindowCoord(p_min), ToWindowCoord(p_max), col, rounding, flags, thickness);
}

void ImGui::bbe::window::AddRectFilled(const ImVec2& p_min, const ImVec2& p_max, float rounding, ImDrawFlags flags)
{
	ImGui::GetForegroundDrawList()->AddRectFilled(ToWindowCoord(p_min), ToWindowCoord(p_max), col, rounding, flags);
}

void ImGui::bbe::window::AddQuad(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, float thickness)
{
	ImGui::GetForegroundDrawList()->AddQuad(ToWindowCoord(p1), ToWindowCoord(p2), ToWindowCoord(p3), ToWindowCoord(p4), col, thickness);
}

void ImGui::bbe::window::AddQuadFilled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4)
{
	ImGui::GetForegroundDrawList()->AddQuadFilled(ToWindowCoord(p1), ToWindowCoord(p2), ToWindowCoord(p3), ToWindowCoord(p4), col);
}

void ImGui::bbe::window::AddTriangle(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, float thickness)
{
	ImGui::GetForegroundDrawList()->AddTriangle(ToWindowCoord(p1), ToWindowCoord(p2), ToWindowCoord(p3), col, thickness);
}

void ImGui::bbe::window::AddTriangleFilled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3)
{
	ImGui::GetForegroundDrawList()->AddTriangleFilled(ToWindowCoord(p1), ToWindowCoord(p2), ToWindowCoord(p3), col);
}

void ImGui::bbe::window::AddCircle(const ImVec2& center, float radius, int num_segments, float thickness)
{
	ImGui::GetForegroundDrawList()->AddCircle(ToWindowCoord(center), ToResolution(radius), col, num_segments, thickness);
}

void ImGui::bbe::window::AddCircleFilled(const ImVec2& center, float radius, int num_segments)
{
	ImGui::GetForegroundDrawList()->AddCircleFilled(ToWindowCoord(center), ToResolution(radius), col, num_segments);
}

void ImGui::bbe::window::AddNgon(const ImVec2& center, float radius, int num_segments, float thickness)
{
	ImGui::GetForegroundDrawList()->AddNgon(ToWindowCoord(center), ToResolution(radius), col, num_segments, thickness);
}

void ImGui::bbe::window::AddNgonFilled(const ImVec2& center, float radius, int num_segments)
{
	ImGui::GetForegroundDrawList()->AddNgonFilled(ToWindowCoord(center), ToResolution(radius), col, num_segments);
}

void ImGui::bbe::window::AddEllipse(const ImVec2& center, float radius_x, float radius_y, float rot, int num_segments, float thickness)
{
	ImGui::GetForegroundDrawList()->AddEllipse(ToWindowCoord(center), ToResolution(radius_x), ToResolution(radius_y), col, rot, num_segments, thickness);
}

void ImGui::bbe::window::AddEllipseFilled(const ImVec2& center, float radius_x, float radius_y, float rot, int num_segments)
{
	ImGui::GetForegroundDrawList()->AddEllipseFilled(ToWindowCoord(center), ToResolution(radius_x), ToResolution(radius_y), col, rot, num_segments);
}

void ImGui::bbe::window::AddText(const ImVec2& pos, const char* text_begin, const char* text_end)
{
	ImGui::GetForegroundDrawList()->AddText(ToWindowCoord(pos), col, text_begin, text_end);
}

void ImGui::bbe::window::AddText(const ImFont* font, float font_size, const ImVec2& pos, const char* text_begin, const char* text_end, float wrap_width, const ImVec4* cpu_fine_clip_rect)
{
	ImGui::GetForegroundDrawList()->AddText(font, font_size, ToWindowCoord(pos), col, text_begin, text_end, wrap_width, cpu_fine_clip_rect);
}

void ImGui::bbe::window::AddPolyline(const ImVec2* points, int num_points, ImDrawFlags flags, float thickness)
{
	auto p = ToWindowCoord(points, num_points);
	ImGui::GetForegroundDrawList()->AddPolyline(p.getRaw(), num_points, col, flags, thickness);
}

void ImGui::bbe::window::AddConvexPolyFilled(const ImVec2* points, int num_points)
{
	auto p = ToWindowCoord(points, num_points);
	ImGui::GetForegroundDrawList()->AddConvexPolyFilled(p.getRaw(), num_points, col);
}

void ImGui::bbe::window::AddBezierCubic(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, float thickness, int num_segments)
{
	ImGui::GetForegroundDrawList()->AddBezierCubic(ToWindowCoord(p1), ToWindowCoord(p2), ToWindowCoord(p3), ToWindowCoord(p4), col, thickness, num_segments);
}

void ImGui::bbe::window::AddBezierQuadratic(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, float thickness, int num_segments)
{
	ImGui::GetForegroundDrawList()->AddBezierQuadratic(ToWindowCoord(p1), ToWindowCoord(p2), ToWindowCoord(p3), col, thickness, num_segments);
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
	ImGui::Text("%s", s.getRaw());
}

void ImGui::TextColored(const ImVec4& col, const::bbe::String& s)
{
	ImGui::TextColored(col, "%s", s.getRaw());
}
