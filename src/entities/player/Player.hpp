
#pragma once

#include <array>
#include <memory>
#include "../../components/PhysicsComponent.hpp"
#include "../../graphics/SpriteHistory.hpp"
#include "../../graphics/TextureUpdater.hpp"
#include "../../particle/Gravitator.hpp"
#include "../../utils/BitFlags.hpp"
#include "../../utils/QuestCode.hpp"
#include "../../utils/Collider.hpp"
#include "../../graphics/Tutorial.hpp"
#include "../../weapon/Hotbar.hpp"
#include "../packages/Health.hpp"
#include "../packages/Caution.hpp"
#include "Catalog.hpp"
#include "Indicator.hpp"
#include "Wallet.hpp"
#include "PlayerAnimation.hpp"
#include "PlayerController.hpp"
#include "Transponder.hpp"
#include "VisitHistory.hpp"
#include "Piggybacker.hpp"

namespace gui {
class Console;
class InventoryWindow;
} // namespace gui

namespace world {
class Map;
}

namespace automa {
struct ServiceProvider;
}

namespace item {
enum class DropType;
}

namespace player {

float const PLAYER_WIDTH = 20.0f;
float const PLAYER_HEIGHT = 20.0f;
float const head_height{8.f};
float const PLAYER_START_X = 100.0f;
float const PLAYER_START_Y = 100.0f;
float const JUMPBOX_HEIGHT = 8.0f;
float const DETECTOR_WIDTH = 8.0f;
float const DETECTOR_HEIGHT = 22.0f;
float const WALL_SLIDE_DETECTOR_OFFSET = 20.0f;
float const DETECTOR_BUFFER = (PLAYER_HEIGHT - DETECTOR_HEIGHT) / 2;
int const JUMP_BUFFER_TIME = 12;
int const INVINCIBILITY_TIME = 200;
int const ANCHOR_BUFFER = 50;
int const num_sprites{220};

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
	sf::Vector2<float> maximum_velocity{};
	float mass{};
	float vertical_dash_multiplier{};
	float dash_speed{};
	float dash_dampen{};
	float wallslide_speed{};
	float antenna_friction{0.93f};
};

struct Counters {
	int invincibility{};
};

enum class State { killed, dir_switch, show_weapon, impart_recoil, crushed};
enum class Triggers { hurt };

struct PlayerFlags {
	util::BitFlags<State> state{};
	util::BitFlags<Triggers> triggers{};
};

class Player {
  public:
	Player(automa::ServiceProvider& svc);
	~Player() {}

	// init (violates RAII but must happen after resource path is set)
	void init(automa::ServiceProvider& svc);
	// member functions
	void update(world::Map& map, gui::Console& console, gui::InventoryWindow& inventory_window);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> campos);
	void render_indicators(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void assign_texture(sf::Texture& tex);
	void update_animation();
	void update_sprite();
	void handle_turning();
	void update_transponder(gui::Console& console, gui::InventoryWindow& inventory_window);
	void flash_sprite();
	void calculate_sprite_offset();
	void set_idle();
	void piggyback(int id);

	// state
	[[nodiscard]] auto alive() const -> bool { return !health.is_dead(); }
	[[nodiscard]] auto is_dead() const -> bool { return health.is_dead(); }
	[[nodiscard]] auto death_animation_over() -> bool { return animation.death_over(); }
	[[nodiscard]] auto just_died() const -> bool { return flags.state.test(State::killed); }
	[[nodiscard]] auto height() const -> float { return collider.dimensions.y; }
	[[nodiscard]] auto width() const -> float { return collider.dimensions.x; }
	[[nodiscard]] auto arsenal_size() const -> size_t { return arsenal ? arsenal.value().size() : 0; }
	[[nodiscard]] auto quick_direction_switch() const -> bool { return flags.state.test(State::dir_switch); }
	[[nodiscard]] auto shielding() -> bool { return controller.get_shield().is_shielding(); }
	[[nodiscard]] auto pushing() const -> bool { return animation.state == AnimState::push || animation.state == AnimState::between_push; }
	[[nodiscard]] auto has_shield() const -> bool { return catalog.categories.abilities.has_ability(Abilities::shield); }
	[[nodiscard]] auto has_item(int id) const -> bool { return catalog.categories.inventory.has_item(id); }
	[[nodiscard]] auto invincible() const -> bool { return health.invincible(); }
	[[nodiscard]] auto has_map() const -> bool { return catalog.categories.inventory.has_item(16); }
	[[nodiscard]] auto moving_left() const -> bool { return directions.movement.lr == dir::LR::left; }
	[[nodiscard]] auto switched_weapon() const -> bool { return hotbar->switched(); }

	// moves
	void jump(world::Map& map);
	void dash();
	void wallslide();
	void shield();

	void set_position(sf::Vector2<float> new_pos, bool centered = false);
	void freeze_position();
	void update_direction();
	void update_weapon();
	void walk();
	void hurt(float amount = 1.f, bool force = false);
	void on_crush(world::Map& map);
	void update_antennae();
	void sync_antennae();

	bool grounded() const;
	bool fire_weapon();

	// level events
	void update_invincibility();
	void start_over();
	void give_drop(item::DropType type, float value);
	void give_item(int item_id, int amount);
	void take_item(int item_id, int amount = 1);
	void equip_item(ApparelType type, int item_id);
	void unequip_item(ApparelType type, int item_id);
	void add_to_hotbar(int id);
	void remove_from_hotbar(int id);

	void reset_flags();
	void total_reset();
	void map_reset();

	arms::Weapon& equipped_weapon();
	void push_to_loadout(int id, bool from_save = false);
	void pop_from_loadout(int id);

	// map helpers
	dir::LR entered_from() const;

	// for debug mode
	std::string print_direction(bool lr);

	//for ledge testing
	entity::Caution caution{};

	// components
	PlayerController controller;
	Transponder transponder{};
	shape::Collider collider{};
	shape::Shape hurtbox{};
	PlayerAnimation animation;
	entity::Health health{};
	Wallet wallet{};
	Indicator health_indicator;
	Indicator orb_indicator;

	text::Tutorial tutorial;

	// weapons
	std::optional<arms::Arsenal> arsenal{};
	std::optional<arms::Hotbar> hotbar{};

	sf::Vector2<float> anchor_point{};
	sf::Vector2<float> sprite_offset{10.f, -3.f};
	sf::Vector2<float> sprite_dimensions{};
	sf::Vector2<float> sprite_position{};

	std::vector<vfx::Gravitator> antennae{};
	sf::Vector2<float> antenna_offset{6.f, -17.f};

	PlayerStats player_stats{0.06f};
	PhysicsStats physics_stats{};
	PlayerFlags flags{};
	util::Cooldown hurt_cooldown{}; //for animation
	util::Cooldown force_cooldown{}; //for player hurt forces
	struct {
		util::Cooldown tutorial{400};
		util::Cooldown sprint_tutorial{800};
		util::Cooldown push{32};
	} cooldowns{};
	Counters counters{};
	std::vector<sf::Vector2<float>> accumulated_forces{};
	std::optional<util::QuestCode> quest_code{};
	std::optional<Piggybacker> piggybacker{};

	automa::ServiceProvider* m_services;

	// sprites
	sf::Sprite sprite;
	flfx::SpriteHistory sprite_history{};

	// texture updater
	flfx::TextureUpdater texture_updater{};

	bool grav = true;

	bool start_cooldown{};
	bool sprite_flip{};

	int ledge_height{}; // temp for testing

	Catalog catalog{};
	VisitHistory visit_history{};

  private:
	struct {
		float stop{5.8f};
		float wallslide{-1.5f};
		float suspend{0.9f};
		float landed{0.4f};
		float run{0.02f};
		float quick_turn{0.9f};
	} thresholds{};
	struct {
		dir::Direction left_squish{};
		dir::Direction right_squish{};
		dir::Direction movement{};
	} directions{};
};

} // namespace player
