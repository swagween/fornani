
#include "fornani/entities/packages/Health.hpp"
#include "fornani/service/ServiceProvider.hpp"

#include <ccmath/ext/clamp.hpp>

namespace fornani::entity {

void Health::set_max(float amount) {
	max_hp = amount;
	hp = amount;
}

void Health::set_hp(float amount) { hp = amount; }

void Health::set_invincibility(float amount) { invincibility_time = static_cast<int>(amount); }

void Health::update() {
	hp = ccm::ext::clamp(hp, 0.f, max_hp);
	invincibility.update();
	taken.update();
	restored.update();
	if (taken.running()) {
		if (taken_point > hp && taken.get_count() > taken_time && taken.get_count() % 32 == 0) { --taken_point; }
		if (taken_point == hp) { taken.cancel(); }
	}
}

void Health::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	drawbox.setFillColor(svc.styles.colors.dark_orange);
	drawbox.setSize({max_hp, 4});
	win.draw(drawbox);
	drawbox.setFillColor(svc.styles.colors.green);
	drawbox.setSize({hp, 4});
	win.draw(drawbox);
};

void Health::heal(float amount) {
	hp += amount;
	restored.start();
}

void Health::refill() {
	hp = max_hp;
	restored.start();
}

void Health::inflict(float amount, bool force) {
	if (invincibility.is_complete() || force) {
		taken_point = hp;
		taken.start();
		hp -= amount;
		invincibility.start(invincibility_time);
		flags.set(HPState::hit);
	}
}

void Health::increase_max_hp(float amount) { set_max(max_hp + amount); }

void Health::reset() { hp = max_hp; }

void Health::debug() {
	ImGui::SliderFloat("hp", &hp, 1.f, max_hp);
	ImGui::SliderFloat("max", &max_hp, 3.f, 20.f);
}

} // namespace fornani::entity
