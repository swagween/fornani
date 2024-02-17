
#pragma once

#include "../../utils/Shape.hpp"

namespace entity {

const uint32_t CELL_SIZE = 32;

class Portal {

  public:
	using Vec = sf::Vector2<float>;
	using Vecu16 = sf::Vector2<uint32_t>;

	Portal() = default;
	Portal(Vecu16 dim, Vecu16 pos) : scaled_dimensions(dim), scaled_position(pos) {
		dimensions = static_cast<Vec>(dim * CELL_SIZE);
		position = static_cast<Vec>(pos * CELL_SIZE);
		bounding_box = shape::Shape(dimensions);
		bounding_box.set_position(position);
	}
	void update();
	void render(sf::RenderWindow& win, Vec campos); // for debugging
	void handle_activation(int room_id, bool& fade_out, bool& done);

	Vec dimensions{};
	Vec position{};
	Vecu16 scaled_dimensions{};
	Vecu16 scaled_position{};
	shape::Shape bounding_box{};

	int destination_map_id{}; // where to send the player
	int source_map_id{};	  // where to place the player once they arrive (check all portals in the destination until you match)

	bool activated{};
	bool activate_on_contact{};
	bool ready{}; // starts false, made true once player isn't intesecting it. to prevent auto-portal for activat-on-contact portals
};

} // namespace entity
