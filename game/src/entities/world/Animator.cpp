
#include <fornani/entities/world/Animator.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::entity {

Animator::Animator(automa::ServiceProvider& svc, std::string_view label, int lookup, sf::Vector2i pos, bool foreground)
	: Animatable(svc, "animators_" + std::string{label}, constants::i_resolution_vec), IWorldPositionable(sf::Vector2u{pos}) {
	set_channel(lookup);
	set_parameters({0, 6, 32, -1});
}

void Animator::update() { tick(); }

void Animator::render(sf::RenderWindow& win, sf::Vector2f cam) {
	set_position(get_world_position() - cam);
	win.draw(*this);
}

} // namespace fornani::entity
