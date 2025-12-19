
#pragma once

#include <fornani/audio/Soundboard.hpp>
#include <fornani/entities/Entity.hpp>
#include <fornani/entities/Mobile.hpp>
#include <fornani/entities/animation/StateMachine.hpp>
#include <fornani/entities/packages/FloatingPart.hpp>
#include <fornani/entities/packages/Health.hpp>
#include <fornani/entities/packages/WeaponPackage.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/graphics/Indicator.hpp>
#include <fornani/gui/HealthBar.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <fornani/utils/Math.hpp>
#include <fornani/utils/Polymorphic.hpp>
#include <fornani/utils/StateFunction.hpp>
#include <string_view>

namespace fornani::player {
class Player;
}

namespace fornani::world {
class Map;
}

namespace fornani::arms {
class Projectile;
}

namespace fornani::enemy {

enum class EnemyChannel { standard, hurt_1, hurt_2, invincible, extra_1, extra_2 };
enum class EnemySize { tiny, small, medium, large, giant };
enum class GeneralFlags {
	mobile,
	gravity,
	player_collision,
	hurt_on_contact,
	map_collision,
	post_death_render,
	no_loot,
	custom_sounds,
	uncrushable,
	foreground,
	spawned,
	transcendent,
	rare_drops,
	permadeath,
	has_invincible_channel,
	invincible_secondary,
	spike_collision,
	custom_channels,
	semipermanent
};
enum class StateFlags { alive, alert, hostile, shot, vulnerable, hurt, shaking, special_death_mode, invisible, advance, no_shake, out_of_zone, no_slowdown, intangible, health_exposed };
enum class Triggers { hostile, alert };
enum class Variant { beast, soldier, elemental, worker, guardian };

struct Attributes {
	float base_hp{};
	float base_damage{};
	float speed{};
	float loot_multiplier{};
	sf::Vector2<int> drop_range{};
	int rare_drop_id{};
	int respawn_distance{};
	float gravity{};
	EnemySize size{EnemySize::medium};
};

struct Flags {
	util::BitFlags<GeneralFlags> general{};
	util::BitFlags<StateFlags> state{};
	util::BitFlags<Triggers> triggers{};
};

class Enemy : public Mobile {
  public:
	Enemy(automa::ServiceProvider& svc, world::Map& map, std::string_view label, bool spawned = false, int variant = 0, sf::Vector2<int> start_direction = {-1, 0});

	void set_external_id(std::pair<int, sf::Vector2<int>> code);

	virtual void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	virtual void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	virtual void gui_render([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] sf::RenderWindow& win, [[maybe_unused]] sf::Vector2f cam) {};
	void post_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;

	void render_indicators(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);

	void handle_player_collision(player::Player& player) const;
	void handle_collision(shape::Collider& other);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj);
	void on_crush(world::Map& map);
	bool seek_home(world::Map& map);
	void set_channel(EnemyChannel to) { Animatable::set_channel(static_cast<int>(to)); }

	[[nodiscard]] auto is_hostile() const -> bool { return flags.state.test(StateFlags::hostile); }
	[[nodiscard]] auto is_alert() const -> bool { return flags.state.test(StateFlags::alert); }
	[[nodiscard]] auto is_hurt() const -> bool { return hurt_effect.running(); }
	[[nodiscard]] auto hostility_triggered() const -> bool { return flags.triggers.test(Triggers::hostile); }
	[[nodiscard]] auto alertness_triggered() const -> bool { return flags.triggers.test(Triggers::alert); }
	[[nodiscard]] auto get_attributes() const -> Attributes { return attributes; }
	[[nodiscard]] auto get_flags() const -> Flags { return flags; }
	[[nodiscard]] auto get_external_id() const -> int { return metadata.external_id; }
	[[nodiscard]] auto has_secondary_collider() const -> bool { return secondary_collider.has_value(); }
	[[nodiscard]] auto get_secondary_collider() const -> shape::Collider& { return secondary_collider.value().get().get_reference(); }
	[[nodiscard]] auto died() const -> bool { return health.is_dead(); }
	[[nodiscard]] auto just_died() const -> bool { return health.is_dead() && post_death.get() == afterlife; }
	[[nodiscard]] auto gone() const -> bool { return post_death.is_complete(); }
	[[nodiscard]] auto player_collision() const -> bool { return flags.general.test(GeneralFlags::player_collision); }
	[[nodiscard]] auto has_map_collision() const -> bool { return flags.general.test(GeneralFlags::map_collision); }
	[[nodiscard]] auto spawn_loot() const -> bool { return !flags.general.test(GeneralFlags::no_loot); }
	[[nodiscard]] auto is_foreground() const -> bool { return flags.general.test(GeneralFlags::foreground); }
	[[nodiscard]] auto is_transcendent() const -> bool { return flags.general.test(GeneralFlags::transcendent); }
	[[nodiscard]] auto permadeath() const -> bool { return flags.general.test(GeneralFlags::permadeath); }

	void intangible_start(int time) { intangibility.start(time); }
	void set_position(sf::Vector2f pos) {
		get_collider().physics.position = pos;
		get_collider().sync_components();
		health_indicator.set_position(pos);
	}
	void cancel_shake() {
		energy = {};
		m_random_offset = {};
	}

	void set_position_from_scaled(sf::Vector2f pos);
	void hurt(float amount = 1.f);
	void shake() { energy = hit_energy; }
	void stop_shaking() { flags.state.reset(StateFlags::shaking); }

	entity::Health health;
	graphics::Indicator health_indicator;

	void debug();

  protected:
	std::string label{};
	std::optional<shape::RegisteredCollider> secondary_owned_collider;
	std::optional<std::reference_wrapper<shape::RegisteredCollider>> secondary_collider;
	Flags flags{};
	Attributes attributes{};
	util::Cooldown post_death{};
	util::Cooldown hitstun{};
	util::Cooldown impulse{};
	util::Cooldown intangibility{};
	int afterlife{200};

	util::Cooldown hurt_effect{};

	EnemyChannel m_custom_channel{};

	struct {
		int id{};
		int variant{};
		int external_id{};
	} metadata{};

	struct {
		std::vector<shape::Shape> hurtbox_atlas{};
		std::vector<shape::Shape> hurtboxes{};
		shape::Shape alert_range{};
		shape::Shape hostile_range{};
		shape::Shape home_detector{};
	} physical{};

	struct {
		int effect_type{};
		int effect_size{};
	} visual;

	struct {
		audio::Enemy hit_flag{};
		util::Cooldown hurt_sound_cooldown{24};
	} sound{};

	// shake
	float energy{};
	float dampen{0.1f};
	float hit_energy{6.f};

	fornani::io::Logger m_logger{"Enemy"};

  private:
	sf::Vector2f m_random_offset{};
	sf::Vector2f m_native_offset{};
	gui::HealthBar m_health_bar;
};

} // namespace fornani::enemy
