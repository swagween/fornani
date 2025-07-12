
#pragma once

#include "fornani/components/PhysicsComponent.hpp"
#include "fornani/components/SteeringBehavior.hpp"
#include "fornani/entities/item/Drop.hpp"
#include "fornani/entities/packages/Caution.hpp"
#include "fornani/entities/packages/Health.hpp"
#include "fornani/entities/player/Catalog.hpp"
#include "fornani/entities/player/Piggybacker.hpp"
#include "fornani/entities/player/PlayerAnimation.hpp"
#include "fornani/entities/player/PlayerController.hpp"
#include "fornani/entities/player/VisitHistory.hpp"
#include "fornani/entities/player/Wallet.hpp"
#include "fornani/graphics/Indicator.hpp"
#include "fornani/graphics/SpriteHistory.hpp"
#include "fornani/graphics/TextureUpdater.hpp"
#include "fornani/gui/WardrobeWidget.hpp"
#include "fornani/io/Logger.hpp"
#include "fornani/particle/Gravitator.hpp"
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Collider.hpp"
#include "fornani/utils/QuestCode.hpp"
#include "fornani/weapon/Hotbar.hpp"

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

constexpr inline float antenna_force{0.18f};
constexpr inline float antenna_speed{336.f};

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
	float antenna_friction{0.93f};
	float roll_speed{};
	float slide_speed{};
};

struct Counters {
	int invincibility{};
};

enum class State { killed, dir_switch, show_weapon, impart_recoil, crushed, sleep, wake_up, busy };
enum class Triggers { hurt };

struct PlayerFlags {
	util::BitFlags<State> state{};
	util::BitFlags<Triggers> triggers{};
};

class Player {
  public:
	Player(automa::ServiceProvider& svc);

	friend class PlayerController;
	friend class PlayerAnimation;

	// member functions
	void update(world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam, sf::Vector2f forced_position);
	void render_indicators(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void assign_texture(sf::Texture& tex);
	void update_animation();
	void update_sprite();
	void handle_turning();
	void flash_sprite();
	void calculate_sprite_offset();
	void set_idle();
	void piggyback(int id);

	// state
	[[nodiscard]] auto alive() const -> bool { return !health.is_dead(); }
	[[nodiscard]] auto is_dead() const -> bool { return health.is_dead(); }
	[[nodiscard]] auto is_busy() const -> bool { return flags.state.test(State::busy); }
	[[nodiscard]] auto death_animation_over() -> bool { return animation.death_over(); }
	[[nodiscard]] auto just_died() const -> bool { return flags.state.test(State::killed); }
	[[nodiscard]] auto height() const -> float { return collider.dimensions.y; }
	[[nodiscard]] auto width() const -> float { return collider.dimensions.x; }
	[[nodiscard]] auto arsenal_size() const -> std::size_t { return arsenal ? arsenal.value().size() : 0; }
	[[nodiscard]] auto quick_direction_switch() const -> bool { return flags.state.test(State::dir_switch); }
	[[nodiscard]] auto shielding() -> bool { return controller.get_shield().is_shielding(); }
	[[nodiscard]] auto pushing() const -> bool { return animation.is_state(AnimState::push) || animation.is_state(AnimState::between_push); }
	[[nodiscard]] auto has_shield() const -> bool { return catalog.abilities.has_ability(Abilities::shield); }
	[[nodiscard]] auto has_item(int id) const -> bool { return catalog.inventory.has_item(id); }
	[[nodiscard]] auto invincible() const -> bool { return health.invincible(); }
	[[nodiscard]] auto has_map() const -> bool { return catalog.inventory.has_item(16); }
	[[nodiscard]] auto moving_left() const -> bool { return directions.movement.lnr == LNR::left; }
	[[nodiscard]] auto switched_weapon() const -> bool { return hotbar->switched(); }
	[[nodiscard]] auto firing_weapon() -> bool { return controller.shot(); }
	[[nodiscard]] auto get_camera_focus_point() const -> sf::Vector2f { return collider.get_center() + camera_offset; }
	[[nodiscard]] auto get_facing_scale() const -> sf::Vector2f { return controller.facing_left() ? sf::Vector2f{-1.f, 1.f} : sf::Vector2f{1.f, 1.f}; }
	[[nodiscard]] auto is_in_animation(AnimState check) const -> bool { return animation.get_state() == check; }
	[[nodiscard]] auto get_desired_direction() const -> SimpleDirection { return m_directions.desired; }
	[[nodiscard]] auto get_actual_direction() const -> SimpleDirection { return m_directions.actual; }

	void set_desired_direction(SimpleDirection to) { m_directions.desired = to; }

	// moves
	void jump(world::Map& map);
	void dash();
	void wallslide();
	void shield();

	void set_position(sf::Vector2f new_pos, bool centered = false);
	void freeze_position();
	void update_direction();
	void update_weapon();
	void walk();
	void hurt(float amount = 1.f, bool force = false);
	void on_crush(world::Map& map);
	void handle_map_collision(world::Map& map);
	void update_antennae();
	void sync_antennae();

	void set_busy(bool flag) { flag ? flags.state.set(State::busy) : flags.state.reset(State::busy); }

	bool grounded() const;
	bool fire_weapon();

	void update_invincibility();
	void start_over();
	void give_drop(item::DropType type, float value);
	void give_item_by_id(int id, item::ItemType type, int amount);
	void give_item(std::string_view label, item::ItemType type, int amount);
	void add_to_hotbar(int id);
	void remove_from_hotbar(int id);
	void set_outfit(std::array<int, static_cast<int>(ApparelType::END)> to_outfit);
	[[nodiscard]] auto get_outfit() -> std::array<int, static_cast<int>(ApparelType::END)> { return catalog.wardrobe.get(); }

	void reset_flags();
	void total_reset();
	void map_reset();

	arms::Weapon& equipped_weapon();
	void push_to_loadout(int id, bool from_save = false);
	void pop_from_loadout(int id);

	// map helpers
	SimpleDirection entered_from() const;

	// for debug mode
	std::string print_direction(bool lr);

	// for ledge testing
	entity::Caution caution{};

	// components
	PlayerController controller;
	shape::Collider collider{};
	shape::Shape hurtbox{};
	PlayerAnimation animation;
	entity::Health health{};
	Wallet wallet{};
	graphics::Indicator health_indicator;
	graphics::Indicator orb_indicator;

	// weapons
	std::optional<arms::Arsenal> arsenal{};
	std::optional<arms::Hotbar> hotbar{};

	sf::Vector2f camera_offset{};
	sf::Vector2f anchor_point{};
	sf::Vector2f sprite_offset{10.f, -3.f};
	sf::Vector2i m_sprite_dimensions;
	sf::Vector2f sprite_position{};

	std::vector<vfx::Gravitator> antennae{};
	sf::Vector2f antenna_offset{6.f, -17.f};

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
	sf::Sprite sprite;
	graphics::SpriteHistory dash_effect;

	// texture updater
	graphics::TextureUpdater texture_updater{};

	bool grav = true;

	bool start_cooldown{};
	bool sprite_flip{};

	int ledge_height{}; // temp for testing

	Catalog catalog{};
	gui::WardrobeWidget wardrobe_widget;
	VisitHistory visit_history{};

  private:
	void set_facing_direction(SimpleDirection to_direction) { m_directions.desired = to_direction; }

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
		components::SteeringBehavior target{};
		components::PhysicsComponent physics{};
	} m_camera{};

	io::Logger m_logger{"player"};
};

} // namespace fornani::player
