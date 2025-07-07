
#include "fornani/entities/packages/Attack.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/world/Map.hpp"



namespace fornani::entity {

void Attack::update() {
	hit.bounds.setOrigin({hit.bounds.getRadius(), hit.bounds.getRadius()});
	sensor.bounds.setOrigin({sensor.bounds.getRadius(), sensor.bounds.getRadius()});
}

void Attack::set_position(sf::Vector2f position) {
	sensor.bounds.setPosition(position + origin);
	hit.bounds.setPosition(position + hit_offset + origin);
}

void Attack::handle_player(player::Player& player) {
	sensor.within_bounds(player.collider.bounding_box) ? sensor.activate() : sensor.deactivate();
	hit.within_bounds(player.collider.bounding_box) ? hit.activate() : hit.deactivate();
}

void Attack::render(sf::RenderWindow& win, sf::Vector2f cam) {
	sensor.render(win, cam);
	hit.render(win, cam);
}

void Attack::enable() { hit.activate(); }

void Attack::disable() { hit.deactivate(); }

} // namespace entity
