
#pragma once

#include "../particle/Spring.hpp"
#include "../utils/Cooldown.hpp"
#include "../utils/Direction.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace arms {
enum class GrappleTriggers { found, released };
enum class GrappleState { anchored, probing, snaking };
class GrapplingHook {
  public:
	GrapplingHook() = default;
	GrapplingHook(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, player::Player& player);
	void break_free(player::Player& player);
	void render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win, sf::Vector2<float>& campos);

	sf::Vector2<float> probe_velocity(float speed);
	vfx::Spring spring{};
	util::BitFlags<GrappleTriggers> grapple_triggers{};
	util::BitFlags<GrappleState> grapple_flags{};
	dir::Direction probe_direction{};

	sf::Sprite rope{};
};

} // namespace arms
