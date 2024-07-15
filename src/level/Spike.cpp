#include "Spike.hpp"
#include <cmath>
#include "../entities/player/Player.hpp"
#include "../service/ServiceProvider.hpp"
#include "../level/Map.hpp"
#include "../particle/Effect.hpp"
#include <algorithm>

namespace world {

Spike::Spike(automa::ServiceProvider& svc, sf::Vector2<float> position, int lookup) {
	collider = shape::Collider({32.f, 32.f});
	facing.und = (lookup == 255 || lookup == 254) ? dir::UND::up : facing.und;
	facing.und = (lookup == 253 || lookup == 252) ? dir::UND::down : facing.und;
	offset = (facing.und == dir::UND::up) ? sf::Vector2<float>{0.f, 22.f} : facing.und == dir::UND::down ? sf::Vector2<float>{0.f, -22.f} : offset; 
	collider.physics.position = position + offset;
	collider.sync_components();
}

void Spike::update(automa::ServiceProvider& svc, world::Map& map) {}

void Spike::handle_collision(shape::Collider& other) const {
	if (attributes.test(SpikeAttributes::no_collision)) { return; }
	other.handle_collider_collision(collider.bounding_box);
}

void Spike::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (svc.greyblock_mode()) { collider.render(win, cam); }
}

} // namespace world
