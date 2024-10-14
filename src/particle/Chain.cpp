
#include "Chain.hpp"
#include "../service/ServiceProvider.hpp"
#include "../entities/player/Player.hpp"
#include <numbers>

namespace vfx {

Chain::Chain(SpringParameters params, sf::Vector2<float> position, int num_links, bool reversed) : root(position) {
	if (reversed) { grav *= -1.f; }
	for (int i{0}; i < num_links; ++i) { links.push_back(Spring({params})); }
	int ctr{};
	auto sign = reversed ? -1.f : 1.f;
	for (auto& link : links) {
		if (ctr == 0) {
			link.set_anchor(position);
			link.set_bob(link.get_anchor() + sf::Vector2<float>{0.f, params.rest_length * sign});
			link.lock();
		} else {
			link.cousin = &links.at(ctr - 1);
			if (link.cousin) { link.set_anchor(link.cousin.value()->get_bob()); }
			link.set_bob(link.get_anchor() + sf::Vector2<float>{0.f, params.rest_length * sign});
		}
		++ctr;
	}
}

void Chain::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, float dampen) {
	auto external_force = sf::Vector2<float>{};
	auto ctr{0};
	float avg{}; // for deriving the start position (there's a better way)
	for (auto& link : links) {
		if (ctr < links.size() - 1) {
			link.set_bob(links.at(static_cast<std::size_t>(ctr + 1)).get_anchor());
		} else {
			
		}
		if (!link.is_locked()) {
			avg += (link.get_anchor().y - link.get_bob().y) / link.get_params().rest_length;
			if (link.cousin) { link.set_anchor(link.cousin.value()->get_bob()); }
		}
		if (link.sensor.within_bounds(player.collider.bounding_box)) {
			link.sensor.activate();
			external_force = {player.collider.physics.velocity.x * external_dampen * dampen, player.collider.physics.velocity.y * external_dampen * 0.1f * dampen};
		} else {
			link.sensor.deactivate();
		}
		link.update(svc, grav, external_force, !link.is_locked());
		++ctr;
	}
	avg /= links.size() - 1;
	if (svc.ticker.every_x_ticks(500)) {
		//std::cout << "avg: " << avg << "\n";
	}

}

void Chain::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	for (auto& link : links) { link.render(win, cam); }
}

} // namespace vfx