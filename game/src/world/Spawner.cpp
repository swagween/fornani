
#include "fornani/world/Spawner.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/particle/Effect.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::world {

Spawner::Spawner(automa::ServiceProvider& svc, sf::Vector2f position, int enemy_id) : position(position), enemy_id(enemy_id) {
	auto random_offset = random::random_range(0, spawn_timer.get_native_time() * 2);
	spawn_timer.start(random_offset);
}

void Spawner::update(automa::ServiceProvider& svc, Map& map) {
	spawn_timer.update();
	if (spawn_timer.is_complete()) {
		auto random_offset = random::random_vector_float(-16.f, 16.f);
		map.spawn_effect(svc, "medium_flash", position);
		if (map.spawn_counter.get_count() < 9) { map.spawn_enemy(enemy_id, position + random_offset); }
		spawn_timer.start();
	}
}

} // namespace fornani::world
