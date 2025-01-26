
#pragma once

#include <string>
#include "fornani/entities/animation/Animation.hpp"
#include "fornani/utils/Shape.hpp"
#include "fornani/utils/StateFunction.hpp"
#include "fornani/entities/Entity.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace entity {

const sf::Vector2<float> large_animator_offset{16.f, 16.f};
enum class AnimatorAttributes{large, automatic, foreground};

class Animator : public Entity {

  public:

	Animator() = default;
	Animator(automa::ServiceProvider& svc, sf::Vector2<int> pos, int id, bool large, bool automatic = false, bool foreground = false, int style = 0);
	void update(automa::ServiceProvider& svc, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) override;
	int get_frame() const;
	[[nodiscard]] auto foreground() const -> bool { return attributes.test(AnimatorAttributes::foreground); }

	sf::Vector2<float> position{};
	sf::Vector2<int> scaled_position{};
	shape::Shape bounding_box{};
	anim::Animation animation{};

	bool activated{};

	int current_frame{};

	sf::Vector2<int> sprite_dimensions{}; // hardcoding for now

	private:
	int id{};
	anim::Parameters automate{0, 6, 28, -1};
	anim::Parameters moving{1, 4, 28, 0};
	anim::Parameters still{0, 1, 28, -1};
	util::BitFlags<AnimatorAttributes> attributes{};
	sf::Sprite sprite;
};

} // namespace entity
