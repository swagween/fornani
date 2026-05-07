
#pragma once

#include <fornani/graphics/Animatable.hpp>

namespace fornani::item {

struct HeldItem final : public Animatable {
	HeldItem(automa::ServiceProvider& svc, int i, int lookup);
	void update();
	void render(sf::RenderWindow& win, sf::Vector2f where);

	int id{};
};

} // namespace fornani::item
