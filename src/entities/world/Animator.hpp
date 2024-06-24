
#pragma once

#include <string>
#include "../animation/Animation.hpp"
#include "../../utils/Shape.hpp"
#include "../../utils/StateFunction.hpp"
#include "../Entity.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace entity {

const int A_UNIT_SIZE = 32;
const sf::Vector2<float> large_animator_offset{16.f, 16.f};

class Animator : public Entity {

  public:

	Animator() = default;
	Animator(automa::ServiceProvider& svc, sf::Vector2<int> pos, bool large);
	void update(player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) override;
	int get_frame() const;

	sf::Vector2<float> position{};
	sf::Vector2<int> scaled_position{};
	shape::Shape bounding_box{};
	anim::Animation animation{};

	int id{};
	bool large{};
	bool activated{};
	bool automatic{};
	bool foreground{};

	int current_frame{0};

	sf::Vector2<int> sprite_dimensions{}; // hardcoding for now

	private:
	anim::Parameters moving{1, 4, 28, 0};
	anim::Parameters still{0, 1, 1, -1};
};

} // namespace entity
