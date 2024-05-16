
#pragma once

#include "../entities/Entity.hpp"
#include "../utils/Collider.hpp"
#include "../utils/Cooldown.hpp"
#include "../utils/Direction.hpp"
#include "../entities/animation/Animation.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace world {
class Map;
}

namespace arms {
enum class GrenadeFlags { detonated };
int const detonation_time{600};
class Grenade : public shape::Collider, public entity::Entity {
  public:
	Grenade(automa::ServiceProvider& svc, sf::Vector2<float> position, dir::Direction direction);
	void update(automa::ServiceProvider& svc, player::Player& player, world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	[[nodiscard]] auto detonated() const -> bool { return grenade_flags.test(GrenadeFlags::detonated); }
	[[nodiscard]] auto get_damage() const -> float { return damage; }
	bool inside_blast(shape::Shape& test);

	sf::CircleShape blast_indicator{};

  private:
	util::Cooldown detonator{};
	anim::Animation animation{};
	util::BitFlags<GrenadeFlags> grenade_flags{};
	sf::CircleShape blast{};
	float damage{24.f};
};

} // namespace arms
