
#pragma once

#include <string>
#include "../behavior/Animation.hpp"
#include "../../utils/Collider.hpp"

namespace automa {
struct ServiceProvider;
}

namespace item {

	enum class DropType { heart, orb };

	struct DropParameters {
		int level{};
		DropType type{};
		anim::Parameters animation_parameters{};
	};

class Drop {

  public:
	using Vec = sf::Vector2<float>;
	using Vecu16 = sf::Vector2<uint32_t>;

	Drop() = default;
	Drop(automa::ServiceProvider& svc);
	void update();
	void render(sf::RenderWindow& win, Vec campos);
	void set_position(sf::Vector2<float> pos);

  private:
	sf::Vector2<float> drop_dimensions{10.f, 10.f};
	shape::Collider collider{};
	sf::Vector2<float> sprite_dimensions{};
	anim::Animation animation{};
	sf::Sprite sprite{};

	DropParameters parameters{};
};

} // namespace item
