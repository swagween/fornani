#include "Shockwave.hpp"
#include "Shockwave.hpp"
#include "Shockwave.hpp"
#include "../player/Player.hpp"
#include "../../level/Map.hpp"
#include "../../service/ServiceProvider.hpp"
#include <algorithm>

namespace entity {

Shockwave::Shockwave(sf::Vector2<float> speed) : speed(speed) {}

void Shockwave::start() {
	lifetime.start();
	hit.bounds.setPosition(origin);
}

void Shockwave::update(automa::ServiceProvider& svc, world::Map& map) {
	lifetime.update();
	hit.bounds.setOrigin({hit.bounds.getRadius(), hit.bounds.getRadius()});
	if (lifetime.is_complete()) { return; }
	hit.bounds.setPosition(hit.bounds.getPosition() + speed);
	if (svc.ticker.every_x_ticks(70)) { map.effects.push_back(entity::Effect(svc, hit.bounds.getPosition(), {0.f, -1.f}, 0, 0)); }
}

void Shockwave::set_position(sf::Vector2<float> position) { hit.bounds.setPosition(position); }

void Shockwave::handle_player(player::Player& player) { hit.within_bounds(player.collider.bounding_box) ? hit.activate() : hit.deactivate(); }

void Shockwave::render(sf::RenderWindow& win, sf::Vector2<float> cam) { hit.render(win, cam); }

} // namespace entity