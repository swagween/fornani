//
//  SavePoint.hpp
//  entity
//
//

#pragma once

#include "../../utils/Shape.hpp"
#include "../../setup/EnumLookups.hpp"
#include <string>

namespace entity {

	class SavePoint {

	public:

		using Vec = sf::Vector2<float>;
		using Vecu16 = sf::Vector2<uint32_t>;

		SavePoint();

		void update();
		void render(sf::RenderWindow& win, Vec campos);

		void save(); // talk to SaveDataManager to write current progress to save.json

		Vec dimensions{32, 32};
		Vec position{};
		Vecu16 scaled_position{};
		shape::Shape bounding_box{};
		sf::Sprite sprite{};
		bool activated{};
		bool can_activate{ true };

		int id{};

	};

} // end entity

/* SavePoint_hpp */
