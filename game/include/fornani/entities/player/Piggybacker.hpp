
#pragma once

#include <fornani/components/SteeringComponent.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <string_view>

namespace fornani::player {

class Player;

class Piggybacker : public Animatable {
  public:
	Piggybacker(automa::ServiceProvider& svc, std::string_view label, sf::Vector2f position);
	void update(automa::ServiceProvider& svc, Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void set_position(sf::Vector2f position) { m_steering.physics.position = position; }

	[[nodiscard]] auto get_id() const -> int { return m_id; }

  private:
	int m_id{};
	components::SteeringComponent m_steering{};
};

} // namespace fornani::player
