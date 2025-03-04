
#pragma once

#include "fornani/entities/animation/AnimatedSprite.hpp"

namespace fornani::automa {
struct ServiceProvider;
}
namespace fornani::entity {

class Animator {

  public:
	Animator(automa::ServiceProvider& svc, std::string label, sf::Vector2i pos, bool foreground = false);
	void update(automa::ServiceProvider& svc);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	[[nodiscard]] auto is_foreground() const -> bool { return m_foreground; }

  private:
	sf::Vector2f m_position{};
	anim::AnimatedSprite m_sprite;
	bool m_foreground{};
};

} // namespace fornani::entity
