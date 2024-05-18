
#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "../utils/Cooldown.hpp"

namespace vfx {
class ColorFade{
  public:
	ColorFade() = default;
	ColorFade(std::vector<sf::Color> colors, int threshold, int duration);
	void start();
	void update();
	[[nodiscard]] auto color() const -> sf::Color { return current_color; }

  private:
	std::vector<sf::Color> colors{};
	int threshold{};
	sf::Color current_color{};
	int duration{};
	util::Cooldown cooldown{};
};

} // namespace flcolor
