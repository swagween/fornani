//
//  Portal.hpp
//  entity
//
//

#pragma once

#include "../../utils/Shape.hpp"

namespace entity {

	class Portal {

	public:

		using Vec = sf::Vector2<float>;

		Portal() = default;
		Portal(Vec dim, Vec pos) : dimensions(dim), position(pos) { bounding_box = Shape(dim); }
		void render(sf::RenderWindow& win, Vec campos); //for debugging

		Vec dimensions{};
		Vec position{};
		Shape bounding_box{};

		int destination_map_id{}; //where to send the player
		int destination_portal_id{}; //where to place the player once they arrive

		bool activated{};

	};

} // end entity

/* Portal_hpp */
