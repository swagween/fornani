
#include <imgui.h>
#include <fornani/entities/packages/Health.hpp>
#include <algorithm>

namespace fornani::entity {

Health::Health(float max) : m_capacity{max}, m_quantity{max}, m_taken{128} {}

void Health::set_capacity(float amount, bool memory) {
	m_capacity = amount;
	if (!memory) { m_quantity = amount; }
}

void Health::set_quantity(float amount) { m_quantity = amount; }

void Health::add_bonus(float amount) {
	m_quantity += amount;
	bonus = amount;
}

void Health::set_invincibility(float amount) { invincibility_time = static_cast<int>(amount); }

void Health::update() {
	invincibility.update();
	m_taken.update();
	restored.update();
	if (m_taken.running()) {
		if (m_taken.is_almost_complete()) { --taken_point; }
	}
}

void Health::heal(float amount) {
	m_quantity = std::clamp(m_quantity + amount, 0.f, get_capacity());
	restored.start();
}

void Health::refill() {
	m_quantity = get_capacity();
	restored.start();
}

void Health::inflict(float amount, bool force) {
	if (invincibility.is_complete() || force) {
		taken_point = m_quantity;
		m_quantity = std::clamp(m_quantity - amount, 0.f, get_capacity());
		m_taken.start();
		invincibility.start(invincibility_time);
		flags.set(HPState::hit);
	}
}

void Health::increase_capacity(float amount) { set_capacity(m_capacity + amount, true); }

void Health::reset() { m_quantity = get_capacity(); }

void Health::kill() { m_quantity = 0.f; }

void Health::debug() {
	ImGui::SliderFloat("m_quantityf", &m_quantity, 1.f, get_capacity(), "%1.f");
	ImGui::SliderFloat("max", &m_capacity, 3.f, 20.f, "%1.f");
	ImGui::SliderFloat("bonusf", &bonus, 1.f, 3.f, "%1.f");
}

} // namespace fornani::entity
