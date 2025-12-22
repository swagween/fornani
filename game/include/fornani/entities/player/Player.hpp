
#pragma once

#include <fornani/components/PhysicsComponent.hpp>
#include <fornani/components/SteeringBehavior.hpp>
#include <fornani/entities/item/Drop.hpp>
#include <fornani/entities/packages/Caution.hpp>
#include <fornani/entities/packages/Health.hpp>
#include <fornani/entities/player/Catalog.hpp>
#include <fornani/entities/player/Piggybacker.hpp>
#include <fornani/entities/player/PlayerAnimation.hpp>
#include <fornani/entities/player/PlayerController.hpp>
#include <fornani/entities/player/VisitHistory.hpp>
#include <fornani/entities/player/Wallet.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/graphics/Indicator.hpp>
#include <fornani/graphics/SpriteHistory.hpp>
#include <fornani/graphics/TextureUpdater.hpp>
#include <fornani/gui/WardrobeWidget.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/particle/Gravitator.hpp>
#include <fornani/physics/RegisteredCollider.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <fornani/utils/QuestCode.hpp>
#include <fornani/weapon/Hotbar.hpp>
#include <fornani/world/Camera.hpp>

namespace fornani {
class Game;
}

namespace fornani::gui {
class Console;
class InventoryWindow;
} // namespace fornani::gui

namespace fornani::world {
class Map;
}

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {

constexpr sf::Vector2f player_dimensions_v = {20.0f, 20.f};
constexpr float head_height_v{8.f};

constexpr float PLAYER_START_X = 100.0f;
constexpr float PLAYER_START_Y = 100.0f;
constexpr float JUMPBOX_HEIGHT = 8.0f;
constexpr float DETECTOR_WIDTH = 8.0f;
constexpr float DETECTOR_HEIGHT = 22.0f;
constexpr float WALL_SLIDE_DETECTOR_OFFSET = 20.0f;
constexpr float DETECTOR_BUFFER = (player_dimensions_v.x - DETECTOR_HEIGHT) / 2;
constexpr int JUMP_BUFFER_TIME = 12;
constexpr int INVINCIBILITY_TIME = 200;
constexpr int ANCHOR_BUFFER = 50;
constexpr int num_sprites{220};

struct PlayerStats {
	float shield_dampen{0.01f};
};

struct PhysicsStats {
	float grav{};
	float ground_fric{};
	float air_fric{};
	float x_acc{};
	float air_multiplier{};
	float sprint_multiplier{};
	float jump_velocity{};
	float jump_release_multiplier{};
	float hurt_acc{};
	sf::Vector2f maximum_velocity{};
	float mass{};
	float vertical_dash_multiplier{};
	float dash_speed{};
	float dash_dampen{};
	float wallslide_speed{};
	float antenna_friction{0.955f};
	float antenna_force{0.58f};
	float roll_speed{};
	float slide_speed{};
};

struct Counters {
	int invincibility{};
};

enum class State { killed, dir_switch, show_weapon, impart_recoil, crushed, sleep, wake_up, busy, dash_kick };
enum class Triggers { hurt };

struct PlayerFlags {
	util::BitFlags<State> state{};
	util::BitFlags<Triggers> triggers{};
};

struct AbilityUsage {
	util::Counter dash{};
	util::Counter doublejump{};
};

class Player final : public Animatable {
  public:
	Player(automa::ServiceProvider& svc);
	void register_with_map(world::Map& map);
	void unregister_with_map();

	friend class PlayerController;
	friend class PlayerAnimation;
	friend class fornani::Game;

	// member functions
	void update(world::Map& map);
	void simple_update(); // collider-free update
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam, sf::Vector2f forced_position);
	void render_indicators(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void assign_texture(sf::Texture& tex);
	void start_tick();
	void end_tick();

	// animation machine
	void request_animation(AnimState const to) { m_animation_machine.request(to); }
	void force_animation(AnimState const to, std::string_view tag) { m_animation_machine.force(to, tag); }
	void set_animation_flag(AnimTriggers const flag, bool on = true) { on ? m_animation_machine.triggers.set(flag) : m_animation_machine.triggers.reset(flag); }
	void set_sleep_timer() { m_animation_machine.set_sleep_timer(); }
	[[nodiscard]] auto get_elapsed_animation_ticks() const -> int { return animation.get_elapsed_ticks(); }

	void update_animation();
	void update_sprite();
	void handle_turning();
	void flash_sprite();
	void set_idle();
	void set_sleeping();
	void set_direction(Direction to);
	void piggyback(int id);

	// state
	[[nodiscard]] auto alive() const -> bool { return !health.is_dead(); }
	[[nodiscard]] auto is_dead() const -> bool { return health.is_dead(); }
	[[nodiscard]] auto is_busy() const -> bool { return flags.state.test(State::busy); }
	[[nodiscard]] auto is_in_custom_sleep_event() const -> bool { return m_animation_machine.is_sleep_timer_running(); }
	[[nodiscard]] auto death_animation_over() -> bool { return m_animation_machine.death_over(); }
	[[nodiscard]] auto just_died() const -> bool { return flags.state.test(State::killed); }
	[[nodiscard]] auto height() const -> float { return collider.value().get().get_reference().dimensions.y; }
	[[nodiscard]] auto width() const -> float { return collider.value().get().get_reference().dimensions.x; }
	[[nodiscard]] auto get_center() const -> sf::Vector2f { return collider.has_value() ? get_collider().get_center() : m_sprite_position; }
	[[nodiscard]] auto get_position() const -> sf::Vector2f { return collider.has_value() ? get_collider().physics.position : m_sprite_position; }
	[[nodiscard]] auto arsenal_size() const -> std::size_t { return arsenal ? arsenal.value().size() : 0; }
	[[nodiscard]] auto quick_direction_switch() const -> bool { return flags.state.test(State::dir_switch); }
	[[nodiscard]] auto pushing() const -> bool { return m_animation_machine.is_state(AnimState::push) || m_animation_machine.is_state(AnimState::between_push); }
	[[nodiscard]] auto has_item(int id) const -> bool { return catalog.inventory.has_item(id); }
	[[nodiscard]] auto has_item(std::string_view tag) const -> bool { return catalog.inventory.has_item(tag); }
	[[nodiscard]] auto has_item_equipped(int id) const -> bool { return catalog.inventory.has_item_equipped(id); }
	[[nodiscard]] auto invincible() const -> bool { return health.invincible(); }
	[[nodiscard]] auto has_map() const -> bool { return catalog.inventory.has_item(16); }
	[[nodiscard]] auto has_collider() const -> bool { return collider.has_value(); }
	[[nodiscard]] auto moving_left() const -> bool { return directions.movement.lnr == LNR::left; }
	[[nodiscard]] auto switched_weapon() const -> bool { return hotbar->switched(); }
	[[nodiscard]] auto firing_weapon() -> bool { return controller.shot(); }
	[[nodiscard]] auto get_piggyback_socket() const -> sf::Vector2f { return m_piggyback_socket; }
	[[nodiscard]] auto get_camera_position() const -> sf::Vector2f { return m_camera.camera.get_position(); }
	[[nodiscard]] auto get_lantern_position() const -> sf::Vector2f { return m_lighting.physics.position; }
	[[nodiscard]] auto get_camera_focus_point() const -> sf::Vector2f { return collider.value().get().get_reference().get_center() + m_camera.target_point; }
	[[nodiscard]] auto get_facing_scale() const -> sf::Vector2f { return controller.facing_left() ? sf::Vector2f{-1.f, 1.f} : sf::Vector2f{1.f, 1.f}; }
	[[nodiscard]] auto is_in_animation(AnimState check) const -> bool { return m_animation_machine.get_state() == check; }
	[[nodiscard]] auto get_desired_direction() const -> SimpleDirection { return m_directions.desired; }
	[[nodiscard]] auto get_actual_direction() const -> SimpleDirection { return m_directions.actual; }
	[[nodiscard]] auto get_piggybacker_id() const -> int { return piggybacker ? piggybacker->get_id() : 0; }
	[[nodiscard]] bool is_intangible() const;

	void set_desired_direction(SimpleDirection to) { m_directions.desired = to; }

	void set_camera_bounds(sf::Vector2f to_bounds) { m_camera.camera.set_bounds(to_bounds); }
	void force_camera_center() { m_camera.camera.force_center(get_camera_focus_point()); }

	void set_position(sf::Vector2f new_pos, bool centered = false);
	void set_draw_position(sf::Vector2f const to);
	void freeze_position();
	void shake_sprite();
	void update_direction();
	void update_weapon();
	void walk();
	void hurt(float amount = 1.f, bool force = false);
	void on_crush(world::Map& map);
	void handle_map_collision(world::Map& map);
	void update_antennae();
	void sync_antennae();

	void set_busy(bool flag) { flag ? flags.state.set(State::busy) : flags.state.reset(State::busy); }
	void set_flag(State const to_set, bool on = true) { on ? flags.state.set(to_set) : flags.state.reset(to_set); }
	void set_trigger(Triggers const to_set, bool on = true) { on ? flags.triggers.set(to_set) : flags.triggers.reset(to_set); }

	bool grounded() const;
	bool fire_weapon();

	void update_invincibility();
	void update_wardrobe();
	void start_over();
	void give_drop(item::DropType type, float value);
	void give_item_by_id(int id, int amount);
	void give_item(std::string_view label, int amount, bool from_save = false);
	[[nodiscard]] EquipmentStatus equip_item(int id);
	void add_to_hotbar(std::string_view tag);
	void remove_from_hotbar(std::string_view tag);
	void set_outfit(std::array<int, static_cast<int>(ApparelType::END)> to_outfit);
	[[nodiscard]] auto get_outfit() -> std::array<int, static_cast<int>(ApparelType::END)> { return catalog.wardrobe.get(); }

	void reset_flags();
	void total_reset();
	void map_reset();

	arms::Weapon& equipped_weapon();
	void push_to_loadout(std::string_view tag, bool from_save = false);
	void pop_from_loadout(std::string_view tag);

	shape::Collider& get_collider() const { return collider.value().get().get_reference(); }

	// map helpers
	SimpleDirection entered_from() const;

	// for ledge testing
	entity::Caution caution{};

	// components
	PlayerController controller;
	shape::Shape hurtbox{};
	shape::Shape distant_vicinity{};
	entity::Health health;
	Wallet wallet{};
	graphics::Indicator health_indicator;
	graphics::Indicator orb_indicator;

	// weapons
	std::optional<arms::Arsenal> arsenal{};
	std::optional<arms::Hotbar> hotbar{};

	sf::Vector2f anchor_point{};
	sf::Vector2f sprite_offset{10.f, -3.f};

	std::vector<vfx::Gravitator> antennae{};

	PlayerStats player_stats{0.06f};
	PhysicsStats physics_stats{};
	PlayerFlags flags{};
	util::Cooldown hurt_cooldown{};	 // for animation
	util::Cooldown force_cooldown{}; // for player hurt forces
	struct {
		util::Cooldown tutorial{400};
		util::Cooldown sprint_tutorial{800};
		util::Cooldown push{32};
	} cooldowns{};
	Counters counters{};
	std::vector<sf::Vector2f> accumulated_forces{};
	std::optional<util::QuestCode> quest_code{};
	std::optional<Piggybacker> piggybacker{};

	automa::ServiceProvider* m_services;

	// sprites
	graphics::SpriteHistory dash_effect;

	// texture updater
	graphics::TextureUpdater texture_updater{};

	Catalog catalog{};
	gui::WardrobeWidget wardrobe_widget;
	VisitHistory visit_history{};

	[[nodiscard]] auto can_dash_kick() const -> bool;

  private:
	void set_facing_direction(SimpleDirection to_direction) { m_directions.desired = to_direction; }

	PlayerAnimation m_animation_machine;

	[[nodiscard]] auto can_dash() const -> bool;
	[[nodiscard]] auto can_omnidirectional_dash() const -> bool;
	[[nodiscard]] auto can_doublejump() const -> bool;
	[[nodiscard]] auto can_roll() const -> bool;
	[[nodiscard]] auto can_slide() const -> bool;
	[[nodiscard]] auto can_jump() const -> bool;
	[[nodiscard]] auto can_wallslide() const -> bool;
	[[nodiscard]] auto can_walljump() const -> bool;

	struct {
		components::SteeringBehavior steering{};
		components::PhysicsComponent physics{};
	} m_lighting{};

	std::optional<shape::RegisteredCollider> owned_collider;
	std::optional<std::reference_wrapper<shape::RegisteredCollider>> collider;

	struct {
		float stop{5.8f};
		float wallslide{-1.5f};
		float suspend{0.9f};
		float landed{0.4f};
		float run{0.02f};
		float quick_turn{0.9f};
	} thresholds{};

	struct {
		Direction left_squish{};
		Direction right_squish{};
		Direction movement{};
	} directions{};

	struct {
		SimpleDirection desired{};
		SimpleDirection actual{};
	} m_directions{};

	struct {
		Camera camera{};
		sf::Vector2f target_point{};
	} m_camera{};

	sf::Vector2f m_sprite_position{};
	sf::Vector2f m_weapon_socket{};
	sf::Vector2f m_piggyback_socket{};
	std::pair<sf::Vector2f, sf::Vector2f> m_antenna_sockets{};
	util::Cooldown m_sprite_shake;
	util::Cooldown m_hurt_cooldown;

	AbilityUsage m_ability_usage{};

	io::Logger m_logger{"player"};
};

} // namespace fornani::player
