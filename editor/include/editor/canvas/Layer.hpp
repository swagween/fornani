
#pragma once

#include "editor/canvas/Grid.hpp"

namespace pi {

class Layer {
  public:
	Layer(std::uint8_t const o, bool const c, sf::Vector2<std::uint32_t> dim, float p = 1.f, bool const il = false, bool const an = false)
		: grid{dim}, render_order(o), collidable(c), dimensions(dim), parallax(p), ignore_lighting(il), animated(an) {}
	void clear();
	void erase();
	void set_position(sf::Vector2f to_position, float scale);
	Grid grid;
	std::uint8_t render_order{};
	bool collidable{};
	bool active{};
	bool ignore_lighting{};
	bool animated{};
	float parallax{};
	sf::Vector2<std::uint32_t> dimensions{};
	std::string label{};
	char const* label_cstr{};
};

} // namespace pi
