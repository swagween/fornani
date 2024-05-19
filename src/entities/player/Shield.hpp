#pragma once

#include <SFML/Graphics.hpp>
#include "../../utils/BitFlags.hpp"
#include "../../utils/Cooldown.hpp"
#include "../../components/CircleSensor.hpp"
#include "../animation/Animation.hpp"
#include "../packages/Health.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {

enum class ShieldTrigger { shield_up, shield_down };
enum class ShieldState { shielding, wobbling, popping };

class Shield {
  public:
	Shield() = default;
	Shield(automa::ServiceProvider& svc);
	void start();
	void pop();
	void end();
	void update(automa::ServiceProvider& svc);
	void damage(float amount);
	void reset_triggers();
	void reset_all();
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);

	[[nodiscard]] auto is_shielding() const -> bool { return flags.state.test(ShieldState::shielding) || flags.state.test(ShieldState::popping); }
	[[nodiscard]] auto shield_up() const -> bool { return flags.triggers.test(ShieldTrigger::shield_up); }
	[[nodiscard]] auto shield_down() const -> bool { return flags.triggers.test(ShieldTrigger::shield_down); }
	struct {
		util::BitFlags<ShieldTrigger> triggers{};
		util::BitFlags<ShieldState> state{};
	} flags{};

	components::CircleSensor sensor{};
	entity::Health health{};

  private:
	struct {
		int time{2800};
	} stats{};
	util::Cooldown timer{};
	sf::Sprite sprite{};
	anim::Animation animation{};
	sf::Vector2<int> dimensions{};
	anim::Parameters neutral{0, 4, 18, -1};
	anim::Parameters popping{2, 3, 8, 0};
	anim::Parameters wobbling{1, 4, 12, -1};
};

} // namespace player
