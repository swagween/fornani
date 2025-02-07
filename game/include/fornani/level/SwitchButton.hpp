#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string_view>
#include "fornani/setup/EnumLookups.hpp"
#include "fornani/utils/Collider.hpp"
#include "fornani/utils/Cooldown.hpp"
#include "fornani/entities/animation/Animation.hpp"
#include "fornani/entities/animation/AnimatedSprite.hpp"
#include "fornani/utils/StateFunction.hpp"
#define SWITCH_BIND(f) std::bind(&SwitchButton::f, this)

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::arms {
class Projectile;
}

namespace fornani::world {

enum class SwitchType { toggler, permanent, movable, alternator };
enum class SwitchButtonState { unpressed, pressed };

class SwitchButton {
  public:
	SwitchButton(automa::ServiceProvider& svc, sf::Vector2<float> position, int id, int type, Map& map);
	void update(automa::ServiceProvider& svc, Map& map, player::Player& player);
	void handle_collision(shape::Collider& other) const;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj);
	shape::Shape& get_bounding_box() { return collider.bounding_box; }
	shape::Shape& get_hurtbox() { return collider.hurtbox; }
	[[nodiscard]] auto pressed() const -> bool { return external == SwitchButtonState::pressed; }
	[[nodiscard]] auto released() const -> bool { return external == SwitchButtonState::unpressed; }

	fsm::StateFunction state_function = std::bind(&SwitchButton::update_unpressed, this);
	fsm::StateFunction update_unpressed();
	fsm::StateFunction update_shining();
	fsm::StateFunction update_squished();
	fsm::StateFunction update_pressed();
	fsm::StateFunction update_rising();
	bool change_state(SwitchButtonState next, std::string_view tag);
	shape::Collider collider{};

  private:
	int id{};
	sf::Vector2<float> sprite_dimensions{};
	util::Cooldown shine_cooldown{800};
	SwitchType type{};
	SwitchButtonState state{};
	SwitchButtonState external{};
	util::BitFlags<SwitchButtonState> triggers{};
	shape::Shape sensor{};
	anim::AnimatedSprite sprite;
	sf::RectangleShape sensorbox{};
};

} // namespace world