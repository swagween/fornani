//
//  Drop.hpp
//  item
//
//

#pragma once

#include "../../utils/Collider.hpp"
#include "../../components/BehaviorComponent.hpp"
#include "../../setup/LookupTables.hpp"
#include <string>

namespace item {

	class Drop {

	public:

		using Vec = sf::Vector2<float>;
		using Vecu16 = sf::Vector2<uint32_t>;

		Drop() = default;
		Drop(Vec dim, Vec pos);
		void update();
		void render(sf::RenderWindow& win, Vec campos);
		int get_frame();

		shape::Collider collider{};
		components::SimpleBehaviorComponent anim{};
		sf::Sprite sprite{};
		

	};

} // end item

/* Drop_hpp */
