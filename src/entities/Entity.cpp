#include "Entity.hpp"
#include "../service/ServiceProvider.hpp"

namespace entity {

void Entity::update(automa::ServiceProvider& svc, world::Map& map) {
	ent_state.reset(State::flip);
	sf::Vector2<float> right_scale = {-1.0f, 1.0f};
	sf::Vector2<float> left_scale = {1.0f, 1.0f};
	if (direction.lr == dir::LR::left && sprite.getScale() == right_scale) {
		ent_state.set(State::flip);
	}
	if (direction.lr == dir::LR::right && sprite.getScale() == left_scale) {
		ent_state.set(State::flip);
	}
}

void Entity::sprite_flip() {
	//all entities default to left-facing
	auto scl = direction.lr == dir::LR::right ? sf::Vector2<float>{-1.0f, 1.0f} : sf::Vector2<float>{1.0f, 1.0f};
	sprite.setScale(scl);
}
} // namespace entity