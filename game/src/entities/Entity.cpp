#include "fornani/entities/Entity.hpp"

#include <tracy/Tracy.hpp>

#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"

namespace fornani::entity {

Entity::Entity(automa::ServiceProvider& svc) {}

void Entity::update(automa::ServiceProvider& svc, world::Map& map) {
	ZoneScopedN("Entity::update");
	ent_state.reset(State::flip);
	// TODO: Why are we not doing anything with this?
	sf::Vector2 right_scale = {-1.0f, 1.0f};
	sf::Vector2 left_scale = {1.0f, 1.0f};
}

void Entity::sprite_shake(automa::ServiceProvider& svc, int rate, int energy) {
	if (svc.ticker.every_x_ticks(rate)) {
		ZoneScopedN("Entity::sprite_shake");
		auto const f_energy = static_cast<float>(energy);
		auto const randx = util::random::random_range_float(-f_energy, f_energy);
		auto const randy = util::random::random_range_float(-f_energy, f_energy);
		random_offset = sf::Vector2{randx, randy};
	}
}
} // namespace fornani::entity