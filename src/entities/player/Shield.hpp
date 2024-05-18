#pragma once

#include <SFML/Graphics.hpp>
#include "../../utils/BitFlags.hpp"
#include "../../utils/Cooldown.hpp"
#include "../../components/CircleSensor.hpp"
#include "../animation/Animation.hpp"

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
	void end();
	void update();
	void reset_triggers();
	void reset_all();
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);

	[[nodiscard]] auto is_shielding() const -> bool { return flags.state.test(ShieldState::shielding); }
	[[nodiscard]] auto shield_up() const -> bool { return flags.triggers.test(ShieldTrigger::shield_up); }
	[[nodiscard]] auto shield_down() const -> bool { return flags.triggers.test(ShieldTrigger::shield_down); }
	struct {
		util::BitFlags<ShieldTrigger> triggers{};
		util::BitFlags<ShieldState> state{};
	} flags{};
	components::CircleSensor sensor{};

  private:
	struct {
		int time{2800};
	} stats{};
	util::Cooldown timer{};
	sf::Sprite sprite{};
	anim::Animation animation{};
};

} // namespace player
