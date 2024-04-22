#include "Caution.hpp"
#include "Caution.hpp"
#include "../player/Player.hpp"
#include "../../level/Map.hpp"
#include <algorithm>

namespace entity {
void Caution::update() {}

void Caution::avoid_player(player::Player& player) {}

void Caution::avoid_ledges(world::Map& map, shape::Collider& collider, int height) {
	// if velocity will take us off a ledge greater than height, we should set the retreat vector to oppose the velocity
	retreat = collider.detect_ledge_height(map) >= height ? sf::Vector2<float>{10.f, 0.f} : sf::Vector2<float>{0.f, 0.f};
	if (collider.flags.state.test(shape::State::ledge_right)) { retreat.x *= -1; }
}
bool entity::Caution::danger(dir::Direction& direction) const { return (retreat.x < 0.f && direction.lr == dir::LR::right) || (retreat.x > 0.f && direction.lr == dir::LR::left); }
} // namespace entity