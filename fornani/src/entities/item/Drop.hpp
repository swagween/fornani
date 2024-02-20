
#pragma once

#include <string>
#include "../../components/BehaviorComponent.hpp"
#include "../../setup/LookupTables.hpp"
#include "../../utils/Collider.hpp"

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

} // namespace item
