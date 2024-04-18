
#pragma once
#include <SFML/Graphics.hpp>
#include <string_view>
#include <unordered_map>
#include "../../utils/BitFlags.hpp"
#include "../../utils/Cooldown.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace player {
class Indicator {
  public:
	Indicator() = default;
	Indicator(automa::ServiceProvider& svc);
	void update(player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void add(int amount);

  private:
	struct {
		int amount{};
	} variables{};
	sf::Text label{};
	sf::Font font{};
	util::Cooldown addition_limit{};
	int const addition_time{180};
	sf::Vector2<float> offset{0.f, 32.f};
};
} // namespace player
