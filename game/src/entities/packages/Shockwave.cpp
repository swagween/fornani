#include "fornani/entities/packages/Shockwave.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/level/Map.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include <algorithm>

namespace fornani::entity {

Shockwave::Shockwave(sf::Vector2<float> speed) : speed(speed) {
	hit.bounds.setRadius(16.f);
	hit.bounds.setOrigin({8.f, 8.f});
}

void Shockwave::start(int time) {
	lifetime.start(time);
	hit.bounds.setPosition(origin);
	position = origin;
}

void Shockwave::update(automa::ServiceProvider& svc, world::Map& map) {
	lifetime.update();
	if (lifetime.is_complete()) { return; }
	position = position + speed;
	if (svc.ticker.every_x_ticks(50)) {
		map.effects.push_back(entity::Effect(svc, position - hit.bounds.getOrigin(), {0.f, -1.f}, 3, 0));
		hit.bounds.setPosition(position);
	}
}

void Shockwave::set_position(sf::Vector2<float> to_position) { hit.bounds.setPosition(to_position); }

void Shockwave::handle_player(player::Player& player) { hit.within_bounds(player.collider.bounding_box) && lifetime.running() ? hit.activate() : hit.deactivate(); }

void Shockwave::render(sf::RenderWindow& win, sf::Vector2<float> cam) { hit.render(win, cam); }

} // namespace entity
