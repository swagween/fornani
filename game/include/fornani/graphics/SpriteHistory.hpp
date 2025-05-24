
#pragma once

#include <SFML/Graphics.hpp>
#include <deque>

namespace fornani::graphics {

class SpriteHistory {
  public:
	SpriteHistory(int sample_size = 4);
	void update(sf::Sprite next, sf::Vector2<float> position);
	void flush();
	void drag(sf::RenderWindow& win, sf::Vector2<float> cam);
	void set_sample_size(int size) { m_sample_size = size; }

  private:
	int m_sample_size;
	int m_dimness_limit{};
	std::deque<std::pair<sf::Sprite, sf::Vector2<float>>> m_pairs{};
};

} // namespace fornani::graphics
