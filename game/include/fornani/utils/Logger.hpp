
#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <imgui.h>

namespace util {
struct AppLog {
	ImGuiTextBuffer Buf;
	ImGuiTextFilter Filter;
	ImVector<int> LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
	bool AutoScroll;		   // Keep scrolling if already at the bottom.

	AppLog() {
		AutoScroll = true;
		clear();
	}

	void clear() {
		Buf.clear();
		LineOffsets.clear();
		LineOffsets.push_back(0);
	}

	void add_log(char const* fmt, ...) IM_FMTARGS(2) {
		int old_size = Buf.size();
		va_list args;
		va_start(args, fmt);
		Buf.appendfv(fmt, args);
		va_end(args);
		for (int new_size = Buf.size(); old_size < new_size; old_size++)
			if (Buf[old_size] == '\n') LineOffsets.push_back(old_size + 1);
	}

	void draw(char const* title, bool* p_open = NULL) {
		if (!ImGui::Begin(title, p_open)) {
			ImGui::End();
			return;
		}

		// Main window
		ImGui::Text("Console");
		ImGui::Separator();

		if (ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			char const* buf = Buf.begin();
			char const* buf_end = Buf.end();
			if (Filter.IsActive()) {
				for (int line_no = 0; line_no < LineOffsets.Size; line_no++) {
					char const* line_start = buf + LineOffsets[line_no];
					char const* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
					if (Filter.PassFilter(line_start, line_end)) ImGui::TextUnformatted(line_start, line_end);
				}
			} else {
				ImGuiListClipper clipper;
				clipper.Begin(LineOffsets.Size);
				while (clipper.Step()) {
					for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++) {
						char const* line_start = buf + LineOffsets[line_no];
						char const* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
						ImGui::TextUnformatted(line_start, line_end);
					}
				}
				clipper.End();
			}
			ImGui::PopStyleVar();
			if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.0f);
		}
		ImGui::EndChild();
		ImGui::End();
	}
};

class Logger {
  public:
	void add_log(char const* message);
	void write_console(ImVec2 size, ImVec2 pos);

  private:
	AppLog log{};
};

} // namespace util