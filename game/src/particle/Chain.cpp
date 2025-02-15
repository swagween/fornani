
#include "fornani/particle/Chain.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/entities/player/Player.hpp"
#include <numbers>

namespace fornani::vfx {

Chain::Chain(automa::ServiceProvider& svc, SpringParameters params, sf::Vector2<float> position, int num_links, bool reversed) : root(position) {
	for (int i{0}; i < num_links; ++i) { links.push_back(Spring({params})); }
	grav = params.grav;
	int ctr{};
	auto sign = reversed ? -1.f : 1.f;
	for (auto& link : links) {
		if (ctr == 0) {
			link.set_anchor(position);
			link.lock();
		} else {
			link.cousin = &links.at(ctr - 1);
			if (link.cousin) { link.set_anchor(link.cousin.value()->get_bob()); }
		}
		link.set_bob(link.get_anchor() + sf::Vector2<float>{0.f, sign * params.rest_length});
		++ctr;
	}
	intro.start();
}

void Chain::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, float dampen) {
	auto break_out{0};
	while (moving() && break_out < 128 && intro.running()) {
		auto ctr{0};
		for (auto& link : links) {
			if (ctr < links.size() - 1) {
				link.set_bob(links.at(static_cast<std::size_t>(ctr + 1)).get_anchor());
			} else {
			}
			if (!link.is_locked()) {
				if (link.cousin) { link.set_anchor(link.cousin.value()->get_bob()); }
			}
			link.update(svc, grav, {}, !link.is_locked(), ctr == links.size() - 1);
			++ctr;
		}
		++break_out;
	}
	intro.update();

	auto external_force = sf::Vector2<float>{};
	auto ctr{0};
	for (auto& link : links) {
		if (ctr < links.size() - 1) {
			link.set_bob(links.at(static_cast<std::size_t>(ctr + 1)).get_anchor());
		} else {
			
		}
		if (!link.is_locked()) {
			if (link.cousin) { link.set_anchor(link.cousin.value()->get_bob()); }
		}
		if (link.sensor.within_bounds(player.collider.bounding_box)) {
			link.sensor.activate();
			external_force = {player.collider.physics.velocity.x * external_dampen * dampen, player.collider.physics.velocity.y * external_dampen * 0.3f * dampen};
		} else {
			link.sensor.deactivate();
		}
		link.update(svc, grav, external_force, !link.is_locked(), ctr == links.size() - 1);
		++ctr;
	}

}

void Chain::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	for (auto& link : links) { link.render(win, cam); }
}

bool Chain::moving() const {
	if (links.empty()) { return false; }
	return abs(links.at(links.size() - 1).variables.bob_physics.velocity.y) > 0.001f;
}

} // namespace vfx