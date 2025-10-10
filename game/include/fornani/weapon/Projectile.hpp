
#pragma once

#include <fornani/audio/Soundboard.hpp>
#include <fornani/components/CircleSensor.hpp>
#include <fornani/components/PhysicsComponent.hpp>
#include <fornani/core/Common.hpp>
#include <fornani/entities/animation/AnimatedSprite.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/graphics/SpriteHistory.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/particle/Gravitator.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <fornani/utils/CircleCollider.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/Direction.hpp>
#include <fornani/utils/Shape.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::world {
class Map;
}

namespace fornani::player {
class Player;
}

namespace fornani::arms {

class Weapon;
enum class ProjectileType : std::uint8_t { bullet, missile, melee };
enum class RenderType : std::uint8_t { animated, single_sprite, multi_sprite };

enum class ProjectileAttributes : std::uint8_t { persistent, transcendent, constrained, circle, omnidirectional, sine, boomerang, wander, reflect, sprite_flip, sticky };
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

enum class ProjectileState : std::uint8_t { initialized, destruction_initiated, destroyed, whiffed, poof, contact, stuck };

class Projectile : public Animatable {
  public:
	Projectile(automa::ServiceProvider& svc, std::string_view label, int id, Weapon& weapon, bool enemy);
	void update(automa::ServiceProvider& svc, player::Player& player);
	void handle_collision(automa::ServiceProvider& svc, world::Map& map);
	void on_player_hit(player::Player& player);
	void render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win, sf::Vector2f cam);
	void destroy(bool completely, bool whiffed = false);
	void seed(automa::ServiceProvider& svc, sf::Vector2f target = {}, float speed_multiplier = 1.f);
	void register_chunk(std::uint8_t chunk) { m_chunk_id = chunk; }
	void set_position(sf::Vector2f pos);
	void set_team(Team to_team);
	void set_firing_direction(Direction to_direction);
	void multiply(float factor) { variables.damage_multiplier = std::min(variables.damage_multiplier * factor, variables.damage_multiplier * 5.f); }
	void poof();
	void damage_over_time();
	void bounce_off_surface(sf::Vector2i direction);

	[[nodiscard]] auto effect_type() const -> int { return visual.effect_type; }
	[[nodiscard]] auto get_type() const -> ProjectileType { return metadata.type; }
	[[nodiscard]] auto destruction_initiated() const -> bool { return variables.state.test(ProjectileState::destruction_initiated); }
	[[nodiscard]] auto destroyed() const -> bool { return variables.state.test(ProjectileState::destroyed); }
	[[nodiscard]] auto get_damage() const -> float { return metadata.specifications.base_damage * variables.damage_multiplier; }
	[[nodiscard]] auto get_power() const -> float { return static_cast<float>(metadata.specifications.power); }
	[[nodiscard]] auto whiffed() const -> bool { return variables.state.test(ProjectileState::whiffed); }
	[[nodiscard]] auto poofed() const -> bool { return variables.state.test(ProjectileState::poof); }
	[[nodiscard]] auto is_stuck() const -> bool { return variables.state.test(ProjectileState::stuck); }
	[[nodiscard]] auto made_contact() const -> bool { return variables.state.test(ProjectileState::contact); }
	[[nodiscard]] auto get_position() const -> sf::Vector2f { return physical.collider.physics.position; }
	[[nodiscard]] auto get_velocity() const -> sf::Vector2f { return physical.collider.physics.apparent_velocity(); }
	[[nodiscard]] auto get_destruction_point() const -> sf::Vector2f { return variables.destruction_point; }
	[[nodiscard]] auto get_team() const -> Team { return metadata.team; }
	[[nodiscard]] auto get_direction() const -> Direction { return physical.direction; }
	[[nodiscard]] auto get_collider() -> shape::CircleCollider& { return physical.collider; }
	[[nodiscard]] auto can_damage() const -> bool { return damage_timer.is_almost_complete() || !persistent(); }

	[[nodiscard]] auto get_chunk_id() const -> std::uint8_t { return m_chunk_id; }

	[[nodiscard]] auto omnidirectional() const -> bool { return metadata.attributes.test(ProjectileAttributes::omnidirectional); }
	[[nodiscard]] auto transcendent() const -> bool { return metadata.attributes.test(ProjectileAttributes::transcendent); }
	[[nodiscard]] auto constrained() const -> bool { return metadata.attributes.test(ProjectileAttributes::constrained); }
	[[nodiscard]] auto sprite_flip() const -> bool { return metadata.attributes.test(ProjectileAttributes::sprite_flip); }
	[[nodiscard]] auto persistent() const -> bool { return metadata.attributes.test(ProjectileAttributes::persistent); }
	[[nodiscard]] auto boomerang() const -> bool { return metadata.attributes.test(ProjectileAttributes::boomerang); }
	[[nodiscard]] auto reflect() const -> bool { return metadata.attributes.test(ProjectileAttributes::reflect); }
	[[nodiscard]] auto sticky() const -> bool { return metadata.attributes.test(ProjectileAttributes::sticky); }
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
		sf::Vector2<int> dimensions{};
		graphics::SpriteHistory sprite_history{};
		vfx::SpriteRotator rotator{};
		Direction direction{};
	} visual;

	struct {
		audio::Projectile hit{};
	} audio;

	struct {
		Direction direction{};
		sf::Vector2f max_dimensions{};
		shape::CircleCollider collider{4.f};
		std::optional<components::CircleSensor> sensor{};
		components::SteeringBehavior steering{};
	} physical{};

	struct {
		float damage_multiplier{1.f};
		sf::Vector2f fired_point{};
		sf::Vector2f destruction_point{};
		util::BitFlags<ProjectileState> state{};
	} variables{};

	std::uint8_t m_chunk_id{};

	util::Cooldown cooldown{};
	util::Cooldown lifetime{};
	util::Cooldown m_reflected;
	util::Cooldown damage_timer{};
	Weapon* m_weapon;

	io::Logger m_logger{"world"};
};

} // namespace fornani::arms
