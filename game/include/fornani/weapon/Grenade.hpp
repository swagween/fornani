
#pragma once

#include "fornani/entities/Entity.hpp"
#include "fornani/utils/Collider.hpp"
#include "fornani/utils/Cooldown.hpp"
#include "fornani/utils/Direction.hpp"
#include "fornani/entities/animation/Animation.hpp"
#include "fornani/components/CircleSensor.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::world {
class Map;
}

namespace fornani::arms {
enum class GrenadeFlags { detonated };
int const detonation_time{600};
class Grenade : public shape::Collider, public entity::Entity {
  public:
	Grenade(automa::ServiceProvider& svc, sf::Vector2<float> position, dir::Direction direction);
	void update(automa::ServiceProvider& svc, player::Player& player, world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	[[nodiscard]] auto detonated() const -> bool { return grenade_flags.test(GrenadeFlags::detonated); }
	[[nodiscard]] auto get_damage() const -> float { return damage; }

	sf::CircleShape blast_indicator{};
	util::Cooldown detonator{};
	anim::Animation animation{};
	util::BitFlags<GrenadeFlags> grenade_flags{};
	components::CircleSensor sensor{};
	float damage{24.f};
	sf::Sprite sprite;
};

} // namespace arms
