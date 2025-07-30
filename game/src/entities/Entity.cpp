#include "fornani/entities/Entity.hpp"

#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"

namespace fornani::entity {

Entity::Entity(automa::ServiceProvider& svc) {}

void Entity::update(automa::ServiceProvider& svc, world::Map& map) {}

void Entity::sprite_shake(automa::ServiceProvider& svc, int rate, int energy) {
	if (svc.ticker.every_x_ticks(rate)) {
		auto const f_energy = static_cast<float>(energy);
		auto const randx = random::random_range_float(-f_energy, f_energy);
		auto const randy = random::random_range_float(-f_energy, f_energy);
		random_offset = sf::Vector2{randx, randy};
	}
}
} // namespace fornani::entity
