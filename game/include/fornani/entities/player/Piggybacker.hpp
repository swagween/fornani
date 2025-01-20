
#pragma once

#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Cooldown.hpp"
#include "fornani/utils/Counter.hpp"
#include "fornani/particle/Gravitator.hpp"
#include <string_view>

namespace player {
class Player;
class Piggybacker {
  public:
	Piggybacker(automa::ServiceProvider& svc, std::string_view label, sf::Vector2<float> position);
	void update(automa::ServiceProvider& svc, Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void set_position(sf::Vector2<float> position) { gravitator.set_position(position); }
  private:
	vfx::Gravitator gravitator{};
	sf::Sprite sprite;
	sf::Vector2<float> offset{};
};

} // namespace player
