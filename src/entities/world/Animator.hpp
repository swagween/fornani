
#pragma once

#include <string>
#include "../animation/Animation.hpp"
#include "../../utils/Shape.hpp"
#include "../../utils/StateFunction.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace entity {

const uint16_t A_UNIT_SIZE = 32;
const sf::Vector2<uint16_t> large_animator_offset{16, 16};

class Animator {

  public:
	using Vec = sf::Vector2<float>;
	using Vecu16 = sf::Vector2<uint16_t>;

	Animator() = default;
	Animator(automa::ServiceProvider& svc, Vecu16 dim, Vecu16 pos);
	void update(player::Player& player);
	void render(sf::RenderWindow& win, Vec campos);
	int get_frame() const;

	Vec dimensions{};
	Vec position{};
	Vecu16 scaled_dimensions{};
	Vecu16 scaled_position{};
	shape::Shape bounding_box{};
	sf::Sprite sprite{};
	anim::Animation animation{};

	int id{};
	bool activated{};
	bool automatic{};
	bool foreground{};
	bool large{};

	int current_frame{0};

	sf::Vector2<int> sprite_dimensions{}; // hardcoding for now
	sf::Vector2<int> spritesheet_dimensions{1024, 320};

	private:
	anim::Parameters moving{1, 4, 28, 0};
	anim::Parameters still{0, 1, 1, -1};
};

} // namespace entity
