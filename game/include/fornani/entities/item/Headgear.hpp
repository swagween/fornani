
#pragma once

#include <fornani/graphics/Animatable.hpp>

namespace fornani::item {

struct Headgear final : public Animatable {
	Headgear(automa::ServiceProvider& svc, int i, int lookup);
	void update(int frame);
	void render(sf::RenderWindow& win, sf::Vector2f where);

	int id{};
};

} // namespace fornani::item
