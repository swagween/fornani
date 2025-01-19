
#pragma once

#include <memory>
#include <unordered_map>
#include "../components/PhysicsComponent.hpp"
#include "../components/CircleSensor.hpp"
#include "../entities/animation/AnimatedSprite.hpp"
#include "../graphics/SpriteHistory.hpp"
#include "../particle/Emitter.hpp"
#include "../particle/Gravitator.hpp"
#include "../particle/Sparkler.hpp"
#include "../../include/fornani/utils/BitFlags.hpp"
#include "../../include/fornani/utils/Cooldown.hpp"
#include "../../include/fornani/utils/CircleCollider.hpp"
#include "../../include/fornani/utils/Direction.hpp"
#include "../../include/fornani/utils/Random.hpp"
#include "../../include/fornani/utils/Shape.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace arms {

class Weapon;
enum class Team { nani, skycorps, guardian, pioneer, beast };
enum class ProjectileType { bullet, missile, melee };
enum class RenderType { animated, single_sprite, multi_sprite };

enum class ProjectileAttributes { persistent, transcendent, constrained, circle, omnidirectional, sine, boomerang, wander, reflect, sprite_flip };
struct ProjectileSpecifications {
	float base_damage{};
	int power{};
	int lifespan{};
	int lifespan_variance{};
	float speed{};
	float speed_variance{};
	float variance{};
	float stun_time{};
	float knockback{};
	float acceleration_factor{};
	float dampen_factor{};
	float dampen_variance{};
	float gravity{};
	float elasticty{};
};

enum class ProjectileState { initialized, destruction_initiated, destroyed, whiffed, poof, contact };

class Projectile {
  public:
	Projectile(automa::ServiceProvider& svc, std::string_view label, int id, Weapon& weapon, bool enemy);
	void update(automa::ServiceProvider& svc, player::Player& player);
	void handle_collision(automa::ServiceProvider& svc, world::Map& map);
	void on_player_hit(player::Player& player);
	void render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win, sf::Vector2<float> cam);
	void destroy(bool completely, bool whiffed = false);
	void seed(automa::ServiceProvider& svc, sf::Vector2<float> target = {});
	void set_position(sf::Vector2<float> pos);
	void set_team(Team to_team);
	void set_firing_direction(dir::Direction to_direction);
	void multiply(float factor) { variables.damage_multiplier = std::min(variables.damage_multiplier * factor, variables.damage_multiplier * 5.f); }
	void poof();
	void damage_over_time();

	[[nodiscard]] auto effect_type() const -> int { return visual.effect_type; }
	[[nodiscard]] auto get_type() const -> ProjectileType { return metadata.type; }
	[[nodiscard]] auto destruction_initiated() const -> bool { return variables.state.test(ProjectileState::destruction_initiated); }
	[[nodiscard]] auto destroyed() const -> bool { return variables.state.test(ProjectileState::destroyed); }
	[[nodiscard]] auto get_damage() const -> float { return metadata.specifications.base_damage * variables.damage_multiplier; }
	[[nodiscard]] auto get_power() const -> float { return metadata.specifications.power; }
	[[nodiscard]] auto whiffed() const -> bool { return variables.state.test(ProjectileState::whiffed); }
	[[nodiscard]] auto poofed() const -> bool { return variables.state.test(ProjectileState::poof); }
	[[nodiscard]] auto made_contact() const -> bool { return variables.state.test(ProjectileState::contact); }
	[[nodiscard]] auto get_position() const -> sf::Vector2<float> { return physical.physics.position; }
	[[nodiscard]] auto get_velocity() const -> sf::Vector2<float> { return physical.physics.apparent_velocity(); }
	[[nodiscard]] auto get_destruction_point() const -> sf::Vector2<float> { return variables.destruction_point; }
	[[nodiscard]] auto get_team() const -> Team { return metadata.team; }
	[[nodiscard]] auto get_direction() const -> dir::Direction { return physical.direction; }
	[[nodiscard]] auto get_bounding_box() -> shape::Shape& { return physical.bounding_box; }
	[[nodiscard]] auto can_damage() const -> bool { return damage_timer.is_almost_complete() || !persistent(); }

	[[nodiscard]] auto omnidirectional() const -> bool { return metadata.attributes.test(ProjectileAttributes::omnidirectional); }
	[[nodiscard]] auto transcendent() const -> bool { return metadata.attributes.test(ProjectileAttributes::transcendent); }
	[[nodiscard]] auto constrained() const -> bool { return metadata.attributes.test(ProjectileAttributes::constrained); }
	[[nodiscard]] auto sprite_flip() const -> bool { return metadata.attributes.test(ProjectileAttributes::sprite_flip); }
	[[nodiscard]] auto persistent() const -> bool { return metadata.attributes.test(ProjectileAttributes::persistent); }
	[[nodiscard]] auto boomerang() const -> bool { return metadata.attributes.test(ProjectileAttributes::boomerang); }
	[[nodiscard]] auto wander() const -> bool { return metadata.attributes.test(ProjectileAttributes::wander); }

  private:
	struct {
		int id{};
		Team team{};
		ProjectileType type{};
		std::string_view label{};
		ProjectileSpecifications specifications{};
		util::BitFlags<ProjectileAttributes> attributes{};
	} metadata{};

	struct {
		int effect_type{};
		int sprite_index{};
		int num_angles{};
		RenderType render_type{};
		anim::AnimatedSprite sprite;
		sf::Vector2<int> dimensions{};
		flfx::SpriteHistory sprite_history{};
		dir::Direction direction{};
	} visual;

	struct {
		shape::Shape bounding_box{};
		dir::Direction direction{};
		sf::Vector2<float> max_dimensions{};
		shape::CircleCollider collider{4.f};
		components::PhysicsComponent physics{};
		std::optional<components::CircleSensor> sensor{};
		components::SteeringBehavior steering{};
	} physical{};

	struct {
		float damage_multiplier{1.f};
		sf::Vector2<float> fired_point{};
		sf::Vector2<float> destruction_point{};
		util::BitFlags<ProjectileState> state{};
	} variables{};

	util::Cooldown cooldown{};
	util::Cooldown lifetime{};
	util::Cooldown damage_timer{};
	Weapon* m_weapon;
};

} // namespace arms
