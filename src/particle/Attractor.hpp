//
//  Attractor.hpp
//  entity
//
//

#pragma once

#include "../utils/Collider.hpp"
#include <string>

namespace vfx {

	class Attractor {

	public:

		using Vec = sf::Vector2<float>;
		using Vecu16 = sf::Vector2<uint32_t>;

		Attractor() = default;
		Attractor(Vec pos, sf::Color col, float agf, Vec size = {4.f, 4.f});
		void update();
		void set_target_position(Vec new_position);
		void render(sf::RenderWindow& win, Vec campos);

		shape::Collider collider{};
		Vecu16 scaled_position{};
		Vec dimensions{};

		sf::Color color{};
		sf::RectangleShape box{};
		float attractor_gravity_force{};

	};

} // end vfx

/* Attractor_hpp */
