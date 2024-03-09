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
	ImGui::Separator();
	ImGui::Text("Positions");

	ImGui::Text("Bob X: %.2f", hook_bob_position.x); ImGui::SameLine();
	ImGui::Text("Bob Y: %.2f", hook_bob_position.y);

	ImGui::Text("Anchor X: %.2f", hook_anchor_position.x); ImGui::SameLine();
	ImGui::Text("Anchor Y: %.2f", hook_anchor_position.y);

	ImGui::Text("Physics X: %.2f", hook_physics_position.x); ImGui::SameLine();
	ImGui::Text("Physics Y: %.2f", hook_physics_position.y);

	ImGui::Text("Box X: %.2f", hook_projectile_box_position.x); ImGui::SameLine();
	ImGui::Text("Box Y: %.2f", hook_projectile_box_position.y);

	ImGui::Text("Proj Physics X: %.2f", hook_projectile_physics_position.x); ImGui::SameLine();
	ImGui::Text("Proj Physics Y: %.2f", hook_projectile_physics_position.y);

	handle_flags();
}

void Logger::handle_flags() {

	if (svc::tickerLocator.get().every_x_frames(20)) { triggers = {}; }
}

void Logger::print_boolean(std::string label, bool test) {}
} // namespace util