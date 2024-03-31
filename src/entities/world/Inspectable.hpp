
#pragma once

#include <string>
#include <djson/json.hpp>
#include "../../utils/Shape.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace gui {
class Console;
}

namespace entity {

const uint32_t UNIT_SIZE = 32;

class Inspectable {

  public:
	using Vec = sf::Vector2<float>;
	using Vecu16 = sf::Vector2<uint32_t>;

	Inspectable() = default;
	Inspectable(Vecu16 dim, Vecu16 pos, std::string_view key) : scaled_dimensions(dim), scaled_position(pos), key(key) {
		dimensions = static_cast<Vec>(dim * UNIT_SIZE);
		position = static_cast<Vec>(pos * UNIT_SIZE);
		bounding_box = shape::Shape(dimensions);
		bounding_box.set_position(position);
	}
	void update(automa::ServiceProvider& svc, player::Player& player, gui::Console& console, dj::Json& set);
	void render(sf::RenderWindow& win, Vec campos); // for debugging

	Vec dimensions{};
	Vec position{};
	Vecu16 scaled_dimensions{};
	Vecu16 scaled_position{};
	shape::Shape bounding_box{};

	bool activated{};
	bool activate_on_contact{};

	std::string key{};
};

} // namespace entity
