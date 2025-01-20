#include "fornani/utils/Logger.hpp"
#include <imgui.h>


namespace util {

void Logger::add_log(char const* message) {
	std::string msg = message;
	msg += "\n";
	log.add_log(msg.data());
}

void Logger::write_console(ImVec2 size, ImVec2 pos) {
	ImGui::SetNextWindowBgAlpha(0.65f);
	ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(size);
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	if (ImGui::Begin("Console", NULL, window_flags)) { log.draw("Console", NULL); }
	ImGui::End();
}

} // namespace util