
#pragma once

#include <string>
#include "../../components/BehaviorComponent.hpp"
#include "../../utils/Shape.hpp"
#include "../../utils/StateFunction.hpp"

namespace entity {

const uint32_t A_UNIT_SIZE = 32;
const sf::Vector2<uint32_t> large_animator_offset{16, 16};

class Animator {

  public:
	using Vec = sf::Vector2<float>;
	using Vecu16 = sf::Vector2<uint32_t>;

	Animator() = default;
	Animator(Vecu16 dim, Vecu16 pos) : scaled_dimensions(dim), scaled_position(pos) {
		dimensions = static_cast<Vec>(dim * A_UNIT_SIZE);
		position = static_cast<Vec>(pos * A_UNIT_SIZE);
		bounding_box = shape::Shape(dimensions);
		bounding_box.set_position(position);
	}
	void update();
	void render(sf::RenderWindow& win, Vec campos);
	int get_frame();

	Vec dimensions{};
	Vec position{};
	Vecu16 scaled_dimensions{};
	Vecu16 scaled_position{};
	shape::Shape bounding_box{};
	sf::Sprite sprite{};
	components::SimpleBehaviorComponent anim{};
	behavior::Behavior behavior{};

	int id{};
	bool activated{};
	bool automatic{};
	bool foreground{};
	bool large{};

	int current_frame{0};

	sf::Vector2<int> sprite_dimensions{}; // hardcoding for now
	sf::Vector2<int> spritesheet_dimensions{1024, 320};
};

} // namespace entity
