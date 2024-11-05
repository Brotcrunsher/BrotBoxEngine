#pragma once

#include "imgui.h"
#include "../BBE/String.h"
#include "../BBE/List.h"
#include "../BBE/Game.h"
#include "../BBE/BrotTime.h"

namespace ImGui
{
	namespace bbe
	{
		namespace INTERNAL
		{
			void setActiveGame(::bbe::Game* game);
		}

		bool InputText(const char* label, ::bbe::String& s, ImGuiInputTextFlags flags = 0);

		void tooltip(const char* text);
		void tooltip(const ::bbe::String& text);
		bool combo(const char* label, const ::bbe::List<::bbe::String>& selections, int32_t* selection);
		bool clickableText(const char* fmt, ...);
		enum /*non-class*/ SecurityButtonFlags
		{
			SecurityButtonFlags_None                     = 0,
			SecurityButtonFlags_DontShowWOSecurityButton = 1 << 0,
		};
		bool securityButton(const char* text, SecurityButtonFlags flags = SecurityButtonFlags_None);
		bool datePicker(const char* label, ::bbe::TimePoint* time);
		bool timePicker(const char* label, int32_t* hour, int32_t* minute);

		void SetColor(ImU32 col);
		float ToResolution(float val);
		void AddLine(const ImVec2& p1, const ImVec2& p2, float thickness = 1.0f);
		void AddRect(const ImVec2& p_min, const ImVec2& p_max, float rounding = 0.0f, ImDrawFlags flags = 0, float thickness = 1.0f);
		void AddRectFilled(const ImVec2& p_min, const ImVec2& p_max, float rounding = 0.0f, ImDrawFlags flags = 0);
		void AddQuad(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, float thickness = 1.0f);
		void AddQuadFilled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4);
		void AddTriangle(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, float thickness = 1.0f);
		void AddTriangleFilled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3);
		void AddCircle(const ImVec2& center, float radius, int num_segments = 0, float thickness = 1.0f);
		void AddCircleFilled(const ImVec2& center, float radius, int num_segments = 0);
		void AddNgon(const ImVec2& center, float radius, int num_segments, float thickness = 1.0f);
		void AddNgonFilled(const ImVec2& center, float radius, int num_segments);
		void AddEllipse(const ImVec2& center, float radius_x, float radius_y, float rot = 0.0f, int num_segments = 0, float thickness = 1.0f);
		void AddEllipseFilled(const ImVec2& center, float radius_x, float radius_y, float rot = 0.0f, int num_segments = 0);
		void AddText(const ImVec2& pos, const char* text_begin, const char* text_end = NULL);
		void AddText(const ImFont* font, float font_size, const ImVec2& pos, const char* text_begin, const char* text_end = NULL, float wrap_width = 0.0f, const ImVec4* cpu_fine_clip_rect = NULL);
		void AddPolyline(const ImVec2* points, int num_points, ImDrawFlags flags, float thickness);
		void AddConvexPolyFilled(const ImVec2* points, int num_points);
		void AddBezierCubic(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, float thickness, int num_segments = 0);
		void AddBezierQuadratic(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, float thickness, int num_segments = 0);
		namespace window
		{
			ImVec2 ToWindowCoord(const ImVec2& p);
			::bbe::List<ImVec2> ToWindowCoord(const ImVec2* points, int num_points);
			void AddLine(const ImVec2& p1, const ImVec2& p2, float thickness = 1.0f);
			void AddRect(const ImVec2& p_min, const ImVec2& p_max, float rounding = 0.0f, ImDrawFlags flags = 0, float thickness = 1.0f);
			void AddRectFilled(const ImVec2& p_min, const ImVec2& p_max, float rounding = 0.0f, ImDrawFlags flags = 0);
			void AddQuad(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, float thickness = 1.0f);
			void AddQuadFilled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4);
			void AddTriangle(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, float thickness = 1.0f);
			void AddTriangleFilled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3);
			void AddCircle(const ImVec2& center, float radius, int num_segments = 0, float thickness = 1.0f);
			void AddCircleFilled(const ImVec2& center, float radius, int num_segments = 0);
			void AddNgon(const ImVec2& center, float radius, int num_segments, float thickness = 1.0f);
			void AddNgonFilled(const ImVec2& center, float radius, int num_segments);
			void AddEllipse(const ImVec2& center, float radius_x, float radius_y, float rot = 0.0f, int num_segments = 0, float thickness = 1.0f);
			void AddEllipseFilled(const ImVec2& center, float radius_x, float radius_y, float rot = 0.0f, int num_segments = 0);
			void AddText(const ImVec2& pos, const char* text_begin, const char* text_end = NULL);
			void AddText(const ImFont* font, float font_size, const ImVec2& pos, const char* text_begin, const char* text_end = NULL, float wrap_width = 0.0f, const ImVec4* cpu_fine_clip_rect = NULL);
			void AddPolyline(const ImVec2* points, int num_points, ImDrawFlags flags, float thickness);
			void AddConvexPolyFilled(const ImVec2* points, int num_points);
			void AddBezierCubic(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, float thickness, int num_segments = 0);
			void AddBezierQuadratic(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, float thickness, int num_segments = 0);
		}
	}

	bool Button(const ::bbe::String& s, const ImVec2& size = ImVec2(0, 0));
	void Text(const ::bbe::String& s);
	void TextColored(const ImVec4& col, const ::bbe::String& s);
}
