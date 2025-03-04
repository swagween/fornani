
#include "fornani/entities/world/Animator.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::entity {

// TODO: get this working again, but JUST for automatic animators. player-interactable animators will be handled by shaders.

Animator::Animator(automa::ServiceProvider& svc, std::string label, sf::Vector2i pos, bool foreground)
	: m_sprite{svc.assets.get_texture(label + "_animators")}, m_position{sf::Vector2f{static_cast<float>(pos.x), static_cast<float>(pos.y)} * svc.constants.cell_size}, m_foreground{foreground} {}

void Animator::update(automa::ServiceProvider& svc) { m_sprite.update(m_position); }

void Animator::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) { m_sprite.render(svc, win, cam); }

} // namespace fornani::entity
