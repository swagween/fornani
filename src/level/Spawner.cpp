#include "Spawner.hpp"
#include <cmath>
#include "../entities/player/Player.hpp"
#include "../service/ServiceProvider.hpp"
#include "../level/Map.hpp"
#include "../particle/Effect.hpp"
#include <algorithm>

namespace world {

Spawner::Spawner(automa::ServiceProvider& svc, sf::Vector2<float> position, int enemy_id) : position(position), enemy_id(enemy_id) { spawn_timer.start(); }

void Spawner::update(automa::ServiceProvider& svc, Map& map) {
	spawn_timer.update();
	if (spawn_timer.is_complete()) {
		auto random_offset = svc.random.random_vector_float(-16.f, 16.f);
		map.effects.push_back(entity::Effect(svc, position + sf::Vector2<float>{16.f, 16.f}, {0.f, 4.f}, 0, 7));
		map.spawn_enemy(enemy_id, position + random_offset);
		spawn_timer.start();
		spawn_count.update();
	}
}

} // namespace world
