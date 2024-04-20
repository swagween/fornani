#include "Health.hpp"
#include "../../service/ServiceProvider.hpp"
#include <algorithm>

namespace entity {

void Health::set_max(float amount) {
	health_states.clear();
	for (auto i = 0; i < amount; ++i) { health_states.push_back(Heart{HPState::filled}); }
	max_hp = amount;
	hp = amount;
}

void Health::set_hp(float amount) { hp = amount; }

void Health::set_invincibility(float amount) { invincibility_time = amount; }

void Health::update() {
	hp = std::clamp(hp, 0.f, max_hp);
	invincibility.update();
	taken.update();
	restored.update();
	auto taken_diff = taken_point - hp;
	if (taken.running()) {
		if (taken_point > hp && taken.get_count() > 200 && taken.get_count() % 32 == 0) { --taken_point; }
		if (taken_point == hp) { taken.cancel(); }
	}
	if (health_states.size() < max_hp) {
		set_max(max_hp);
	} else {
		for (auto i = 0; i < max_hp; ++i) {
			health_states.at(i).state = hp > i ? HPState::filled : taken_point > i ? HPState::taken : HPState::gone;
			auto flashing = restored.running() && restored.get_cooldown() % 48 > 24 && hp > i;
			health_states.at(i).state = flashing ? HPState::light : health_states.at(i).state;
		}
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
	restored.start(128);
}

void Health::inflict(float amount) {
	if (invincibility.is_complete()) {
		taken_point = hp;
		taken.start();
		hp -= amount;
		invincibility.start(invincibility_time);
	}
}

void Health::reset() { hp = max_hp; }

} // namespace entity