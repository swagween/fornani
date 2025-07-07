
#pragma once

#include "fornani/particle/Gravitator.hpp"
#include <string_view>

namespace fornani::player {
class Player;
class Piggybacker {
  public:
	Piggybacker(automa::ServiceProvider& svc, std::string_view label, sf::Vector2f position);
	void update(automa::ServiceProvider& svc, Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void set_position(sf::Vector2f position) { gravitator.set_position(position); }
  private:
	vfx::Gravitator gravitator{};
	sf::Sprite sprite;
	sf::Vector2f offset{};
};

} // namespace fornani::player
