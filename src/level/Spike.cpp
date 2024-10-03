#include "Spike.hpp"
#include <cmath>
#include "../entities/player/Player.hpp"
#include "../service/ServiceProvider.hpp"
#include "../level/Map.hpp"
#include "../particle/Effect.hpp"
#include <algorithm>

namespace world {

Spike::Spike(automa::ServiceProvider& svc, sf::Vector2<float> position, int lookup) : hitbox({32.f, 32.f}) {
	collider = shape::Collider({32.f, 32.f});
	auto adjustment = 22.f;
	facing.und = (lookup == 255 || lookup == 254) ? dir::UND::up : facing.und;
	facing.und = (lookup == 253 || lookup == 252) ? dir::UND::down : facing.und;
	offset.y = (facing.und == dir::UND::up) ? adjustment : facing.und == dir::UND::down ? -adjustment : offset.y;
	facing.lr = (lookup == 251 || lookup == 250) ? dir::LR::left : facing.lr;
	facing.lr = (lookup == 249 || lookup == 248) ? dir::LR::right : facing.lr;
	offset.x = (facing.lr == dir::LR::left) ? adjustment : facing.lr == dir::LR::right ? -adjustment : offset.x;
	collider.physics.position = position + offset;
	collider.sync_components();
	hitbox.set_position(position);
}

void Spike::update(automa::ServiceProvider& svc, player::Player& player, world::Map& map) {
	if (player.hurtbox.overlaps(hitbox)) { player.hurt(); }
	handle_collision(player.collider);
}

void Spike::handle_collision(shape::Collider& other) const {
	if (attributes.test(SpikeAttributes::no_collision)) { return; }
	other.handle_collider_collision(collider.bounding_box);
}

void Spike::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (svc.greyblock_mode()) { collider.render(win, cam); }
}

} // namespace world
