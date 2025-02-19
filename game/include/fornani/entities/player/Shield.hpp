#pragma once

#include <SFML/Graphics.hpp>
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Cooldown.hpp"
#include "fornani/components/CircleSensor.hpp"
#include "fornani/entities/animation/Animation.hpp"
#include "fornani/entities/packages/Health.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {

enum class ShieldTrigger : uint8_t { shield_up, shield_down, hit };
enum class ShieldState : uint8_t { shielding, wobbling, popping, recovery };

class Shield {
  public:
	explicit Shield(automa::ServiceProvider& svc);
	void start();
	void pop();
	void end();
	void update(automa::ServiceProvider& svc);
	void damage(float amount = 0.f);
	void reset_triggers();
	void reset_all();
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);

	[[nodiscard]] auto is_shielding() const -> bool { return flags.state.test(ShieldState::shielding); }
	[[nodiscard]] auto recovering() const -> bool { return flags.state.test(ShieldState::recovery); }
	[[nodiscard]] auto shield_up() const -> bool { return flags.triggers.test(ShieldTrigger::shield_up); }
	[[nodiscard]] auto shield_down() const -> bool { return flags.triggers.test(ShieldTrigger::shield_down); }
	[[nodiscard]] auto active() const -> bool { return flags.state.test(ShieldState::shielding) || flags.state.test(ShieldState::popping); }
	struct {
		util::BitFlags<ShieldTrigger> triggers{};
		util::BitFlags<ShieldState> state{};
	} flags{};

	components::CircleSensor sensor{};
	entity::Health health{};
	float switch_point{};

	struct {
		anim::Parameters shielding{0, 4, 28, -1};
		anim::Parameters not_shielding{0, 1, 28, -1};
		anim::Parameters recovering{3, 2, 36, -1};
	} hud_animations{};
	anim::Animation hud_animation{};

  private:
	struct {
		int time{2800};
		float depletion_rate{0.005f};
		float regen_rate{0.015f};
		float recovery_regen_rate{0.01f};
	} stats{};
	util::Cooldown timer{};
	sf::Sprite sprite;

	anim::Animation animation{};

	sf::Vector2<int> dimensions{};
	anim::Parameters neutral{0, 4, 18, -1};
	anim::Parameters popping{2, 3, 8, 0};
	anim::Parameters wobbling{1, 4, 12, -1};
};

} // namespace player
