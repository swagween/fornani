#include "Shockwave.hpp"
#include "Shockwave.hpp"
#include "Shockwave.hpp"
#include "../player/Player.hpp"
#include "../../level/Map.hpp"
#include "../../service/ServiceProvider.hpp"
#include <algorithm>

namespace entity {

Shockwave::Shockwave(sf::Vector2<float> speed) : speed(speed) { hit.bounds.setRadius(16.f); }

void Shockwave::start(int time) {
	lifetime.start(time);
	hit.bounds.setPosition(origin);
	position = origin;
}

void Shockwave::update(automa::ServiceProvider& svc, world::Map& map) {
	lifetime.update();
	if (lifetime.is_complete()) { return; }
	position = position + speed;
	if (svc.ticker.every_x_ticks(70)) {
		map.effects.push_back(entity::Effect(svc, position, {0.f, -1.f}, 0, 0));
		hit.bounds.setPosition(position);
	}
}

void Shockwave::set_position(sf::Vector2<float> position) { hit.bounds.setPosition(position); }

void Shockwave::handle_player(player::Player& player) { hit.within_bounds(player.collider.bounding_box) && lifetime.running() ? hit.activate() : hit.deactivate(); }

void Shockwave::render(sf::RenderWindow& win, sf::Vector2<float> cam) { hit.render(win, cam); }

} // namespace entity