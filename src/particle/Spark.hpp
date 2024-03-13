
#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include "../utils/Cooldown.hpp"

namespace vfx {

class Spark {
  public:
	Spark(sf::Vector2<float> pos, sf::Color color);
	void update();
	void render(sf::RenderWindow& win,sf::Vector2<float> cam);
	bool done() const;
  private:
	sf::RectangleShape box{};
	sf::Vector2<float> position{};
	util::Cooldown lifespan{};
	int frame{};
	
};

} // namespace vfx
