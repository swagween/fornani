#include "editor/gui/Console.hpp"
#include <imgui.h>

#include <string>
#include "Console.hpp"

namespace pi {

void Console::add_log(char const* message) {
	std::string msg = message;
	msg += "\n";
	log.add_log(msg.data());
}

void Console::write_console(ImVec2 prev_size, ImVec2 prev_pos) {
	auto pad{10.f};
	ImGuiViewport const* viewport = ImGui::GetMainViewport();
	auto port_size = viewport->WorkSize;
	ImGui::SetNextWindowBgAlpha(0.65f);
	ImVec2 work_pos = prev_pos;
	ImVec2 work_size = prev_size;
	work_pos.y += prev_size.y + pad;
	work_size.y = port_size.y - prev_size.y - 3.f * pad;
	ImGui::SetNextWindowPos(work_pos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(work_size);
	ImGuiWindowFlags const window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	if (ImGui::Begin("Console", nullptr, window_flags)) { log.draw("Console", nullptr); }
	ImGui::End();
}

} // namespace pi
