#include "fornani/entities/Entity.hpp"

#include <tracy/Tracy.hpp>

#include "fornani/service/ServiceProvider.hpp"

namespace entity {

Entity::Entity(automa::ServiceProvider& svc) {}

void Entity::update(automa::ServiceProvider& svc, world::Map& map) {
	ZoneScopedN("Entity::update");
	ent_state.reset(State::flip);
	sf::Vector2<float> right_scale = {-1.0f, 1.0f};
	sf::Vector2<float> left_scale = {1.0f, 1.0f};
}

void Entity::sprite_shake(automa::ServiceProvider& svc, int rate, int energy) {
	if (svc.ticker.every_x_ticks(rate)) {
		ZoneScopedN("Entity::sprite_shake");
		auto f_energy = static_cast<float>(energy);
		auto randx = svc.random.random_range_float(-f_energy, f_energy);
		auto randy = svc.random.random_range_float(-f_energy, f_energy);
		random_offset = sf::Vector2<float>{randx, randy};
	}
}
} // namespace entity