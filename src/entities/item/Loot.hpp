//
//  Loot.hpp
//  item
//
//

#pragma once

//#include "../../particle/Emitter.hpp"
#include "Drop.hpp"
#include <string>

namespace item {

	class Loot {

		using Vec = sf::Vector2<float>;
		using Vecu16 = sf::Vector2<uint32_t>;

	public:

		Loot() = default;
		Loot(Vec pos);

		void update();

		//vfx::Emitter burst{};
		std::vector<Drop> drops{};

	};

} // end item

/* Loot_hpp */
