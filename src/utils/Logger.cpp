#include "Logger.hpp"
#include <imgui.h>
#include "../setup/ServiceLocator.hpp"

namespace util {

void Logger::run() {
	ImGui::Text("States");
	ImGui::Text("Projectile Probing: %s", states.test(State::hook_probing) ? "Yes" : "No");
	ImGui::Text("Projectile Anchored: %s", states.test(State::hook_anchored) ? "Yes" : "No");
	ImGui::Text("Projectile Snaking: %s", states.test(State::hook_snaking) ? "Yes" : "No");
	ImGui::Separator();
	ImGui::Text("Triggers");
	ImGui::Text("Projectile Released: %s", triggers.test(Trigger::hook_released) ? "Yes" : "No");
	handle_flags();
}

void Logger::handle_flags() {

	if (svc::tickerLocator.get().every_x_frames(20)) { triggers = {}; }
}

void Logger::print_boolean(std::string label, bool test) {}
} // namespace util