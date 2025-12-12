#pragma once

#include "fornani/components/CircleSensor.hpp"
#include "fornani/entities/animation/AnimatedSprite.hpp"
#include "fornani/entities/packages/Health.hpp"
#include "fornani/particle/Gravitator.hpp"
#include "fornani/physics/Collider.hpp"
#include "fornani/utils/StateFunction.hpp"
#define SPAWNABLE_PLAT_BIND(f) std::bind(&SpawnablePlatform::f, this)

namespace fornani::automa {
struct ServiceProvider;
}
namespace fornani::player {
class Player;
}

namespace fornani::entity {
enum class SpawnablePlatformState { open, opening, fading, closing, dormant };
class SpawnablePlatform {
  public:
	SpawnablePlatform(automa::ServiceProvider& svc, sf::Vector2f position, int index = 0);
	SpawnablePlatform(SpawnablePlatform const& other) : sprite(other.sprite), index(other.index), m_health{1.f} {}

	void update(automa::ServiceProvider& svc, player::Player& player, sf::Vector2f target);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	std::unique_ptr<SpawnablePlatform> clone() const { return std::make_unique<SpawnablePlatform>(*this); }
	[[nodiscard]] auto get_index() const -> int { return index; }
	[[nodiscard]] auto collidable() const -> bool { return state == SpawnablePlatformState::open || state == SpawnablePlatformState::opening || state == SpawnablePlatformState::fading; }

  private:
	fsm::StateFunction state_function = std::bind(&SpawnablePlatform::update_dormant, this);
	fsm::StateFunction update_open();
	fsm::StateFunction update_opening();
	fsm::StateFunction update_fading();
	fsm::StateFunction update_closing();
	fsm::StateFunction update_dormant();
	bool change_state(SpawnablePlatformState next, std::string_view tag);

	SpawnablePlatformState state{};
	shape::Collider collider{};
	int index{};
	vfx::Gravitator gravitator{};
	components::CircleSensor sensor{};
	anim::AnimatedSprite sprite;
	Health m_health;
};
} // namespace fornani::entity
