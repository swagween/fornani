#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string_view>
#include "../setup/EnumLookups.hpp"
#include "../utils/Collider.hpp"
#include "../utils/Cooldown.hpp"
#include "../entities/animation/Animation.hpp"
#include "../entities/animation/AnimatedSprite.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace arms {
class Projectile;
}

namespace world {

enum class SwitchType { toggler, permanent, timed, alternator };
enum class SwitchButtonState { unpressed, squishing, pressed, rising };
enum class SwitchTriggers { on, pressed, released };

class SwitchButton {
  public:
	SwitchButton(automa::ServiceProvider& svc, sf::Vector2<float> position, int id, int type);
	void update(automa::ServiceProvider& svc, Map& map, player::Player& player);
	void handle_collision(shape::Collider& other) const;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj);
	shape::Shape& get_bounding_box() { return collider.bounding_box; }
	shape::Shape& get_hurtbox() { return collider.hurtbox; }
	[[nodiscard]] auto pressed() const -> bool { return triggers.test(SwitchTriggers::pressed); }
	[[nodiscard]] auto released() const -> bool { return triggers.test(SwitchTriggers::released); }

  private:
	int id{};
	sf::Vector2<float> sprite_dimensions{};
	sf::Vector2<float> sprite_offset{0.f, 2.f};
	util::Cooldown released_cooldown{40};
	util::Cooldown pressed_cooldown{40};
	util::BitFlags<SwitchTriggers> triggers{};
	SwitchType type{};
	SwitchButtonState state{};
	shape::Collider collider{};
	shape::Shape sensor{};
	anim::AnimatedSprite sprite;
};
} // namespace world