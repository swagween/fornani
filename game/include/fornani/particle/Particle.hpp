
#pragma once

#include <fornani/core/Fwd.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/physics/RegisteredCollider.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/Fader.hpp>
#include <optional>
#include <string_view>

namespace fornani::vfx {

class Particle {
  public:
	Particle(automa::ServiceProvider& svc, sf::Vector2f pos, sf::Vector2f dim, std::string_view type, sf::Color color, Direction direction, int channel = 0);
	Particle(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f pos, sf::Vector2f dim, std::string_view type, sf::Color color, Direction direction, int channel = 0);
	void update(automa::ServiceProvider& svc, world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void submit(Renderer& renderer);

	[[nodiscard]] auto done() const -> bool { return lifespan.is_almost_complete(); }

  private:
	sf::RectangleShape box{};
	sf::Vector2f position{};
	sf::Vector2f dimensions{};

	util::Cooldown lifespan{};
	std::optional<components::PhysicsComponent> m_physics{};
	std::optional<shape::RegisteredCollider> m_collider{};
	std::optional<Animatable> m_animatable{};
	std::optional<util::Fader> m_fader{};
	int frame{};
};

} // namespace fornani::vfx
