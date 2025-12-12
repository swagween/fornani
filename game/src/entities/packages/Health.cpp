
#include <imgui.h>
#include <fornani/entities/packages/Health.hpp>
#include <algorithm>

namespace fornani::entity {

void Health::set_max(float amount, bool memory) {
	max_hp = amount;
	if (!memory) { hp = amount; }
}

void Health::set_hp(float amount) { hp = amount; }

void Health::set_invincibility(float amount) { invincibility_time = static_cast<int>(amount); }

void Health::update() {
	invincibility.update();
	taken.update();
	restored.update();
	if (taken.running()) {
		if (taken_point > hp && taken.get_count() > taken_time && taken.get_count() % 32 == 0) { --taken_point; }
		if (taken_point == hp) { taken.cancel(); }
	}
}

void Health::heal(float amount) {
	hp = std::clamp(hp + amount, 0.f, max_hp);
	restored.start();
}

void Health::refill() {
	hp = max_hp;
	restored.start();
}

void Health::inflict(float amount, bool force) {
	if (invincibility.is_complete() || force) {
		hp = std::clamp(hp - amount, 0.f, max_hp);
		taken_point = hp;
		taken.start();
		invincibility.start(invincibility_time);
		flags.set(HPState::hit);
	}
}

void Health::increase_max_hp(float amount) { set_max(max_hp + amount, true); }

void Health::reset() { hp = max_hp; }

void Health::kill() { hp = 0.f; }

void Health::debug() {
	ImGui::SliderFloat("hp", &hp, 1.f, max_hp);
	ImGui::SliderFloat("max", &max_hp, 3.f, 20.f);
}

} // namespace fornani::entity
