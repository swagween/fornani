#pragma once
#include <array>
#include <SFML/Graphics.hpp>
#include "Cooldown.hpp"

namespace util {

class NineSlice {
  public:
	std::array<sf::Sprite, 9> sprites{};
	void slice(int corner_factor, int edge_factor);
	void set_texture(sf::Texture& tex);
	void set_origin(sf::Vector2<float> origin);
	void update(sf::Vector2<float> position, sf::Vector2<float> dimensions, int corner_dim, int edge_dim);
	void render(sf::RenderWindow& win) const;
	void start() { appear.start(); }

  private:
	util::Cooldown appear{128};
	float global_scale{};
};

} // namespace util