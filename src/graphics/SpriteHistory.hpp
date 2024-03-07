
#pragma once

#include <SFML/Graphics.hpp>
#include <deque>

namespace flfx {

class SpriteHistory {
  public:
	std::deque<sf::Sprite> sprites{};
	std::deque<sf::Vector2<float>> positions{};
	void update(sf::Sprite next) {
		sprites.push_back(next);
		positions.push_back(next.getPosition());
		if (sprites.size() >= sample_size) {
			sprites.pop_front();
			positions.pop_front();
		}
	}
	void flush() {
		if (sprites.size() > 0 && positions.size() > 0) {
			sprites.pop_front();
			positions.pop_front();
		}
	}
	void set_sample_size(int size) { sample_size = size; }

  private:
	int sample_size{4};

};

} // namespace flfx
