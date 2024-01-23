//
//  Antenna.hpp
//  entity
//
//

#pragma once

#include "../../utils/Collider.hpp"
#include <string>

namespace entity {

	class Antenna {

	public:

		using Vec = sf::Vector2<float>;
		using Vecu16 = sf::Vector2<uint32_t>;

		Antenna() = default;
		Antenna(Vec pos, sf::Color col, float agf);
		void update();
		void set_target_position(Vec new_position);
		void render(sf::RenderWindow& win, Vec campos);

		shape::Collider collider{};
		Vecu16 scaled_position{};

		sf::Color color{};
		sf::RectangleShape box{};
		float antenna_gravity_force{};

	};

} // end entity

/* Antenna_hpp */
