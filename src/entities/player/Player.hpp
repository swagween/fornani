
#pragma once

#include <array>
#include <memory>
#include "../../components/PhysicsComponent.hpp"
#include "../../graphics/SpriteHistory.hpp"
#include "../../graphics/TextureUpdater.hpp"
#include "../../particle/Gravitator.hpp"
#include "../../utils/BitFlags.hpp"
#include "../../utils/Collider.hpp"
#include "../../weapon/Arsenal.hpp"
#include "../packages/Health.hpp"
#include "Catalog.hpp"
#include "Indicator.hpp"
#include "PlayerAnimation.hpp"
#include "PlayerController.hpp"
#include "Transponder.hpp"

namespace gui {
class Console;
class InventoryWindow;
} // namespace gui

namespace automa {
struct ServiceProvider;
}

namespace item {
enum class DropType;
}

namespace player {

float const PLAYER_WIDTH = 18.0f;
float const PLAYER_HEIGHT = 24.0f;
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
constexpr inline float antenna_speed{136.f};

struct PlayerStats {
	int orbs{};
	int max_orbs{};
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
};

struct Counters {
	int invincibility{};
};

enum class State { alive, killed, dir_switch, show_weapon, impart_recoil };
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
	void update(gui::Console& console, gui::InventoryWindow& inventory_window);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> campos);
	void render_indicators(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void assign_texture(sf::Texture& tex);
	void update_animation();
	void update_sprite();
	void handle_turning();
	void update_transponder(gui::Console& console, gui::InventoryWindow& inventory_window);
	void flash_sprite();
	void calculate_sprite_offset();

	// state
	[[nodiscard]] auto is_dead() const -> bool { return flags.state.test(State::alive); }
	[[nodiscard]] auto just_died() const -> bool { return flags.state.test(State::killed); }
	[[nodiscard]] auto height() const -> float { return collider.dimensions.y; }
	[[nodiscard]] auto width() const -> float { return collider.dimensions.x; }
	[[nodiscard]] auto quick_direction_switch() const -> bool { return flags.state.test(State::dir_switch); }
	[[nodiscard]] auto shielding() -> bool { return controller.get_shield().is_shielding(); }
	[[nodiscard]] auto has_shield() const -> bool { return catalog.categories.abilities.has_ability(Abilities::shield); }
	[[nodiscard]] auto invincible() const -> bool { return health.invincible(); }

	// moves
	void jump();
	void dash();
	void wallslide();
	void shield();

	void set_position(sf::Vector2<float> new_pos, bool centered = false);
	void update_direction();
	void update_weapon();
	void walk();
	void hurt(float amount);
	void update_antennae();
	void sync_antennae();

	bool grounded() const;
	bool fire_weapon();

	// level events
	void update_invincibility();
	void kill();
	void start_over();
	void give_drop(item::DropType type, float value);
	void give_item(int item_id, int amount);

	void reset_flags();
	void total_reset();
	void map_reset();

	arms::Weapon& equipped_weapon();
	void push_to_loadout(int id);
	void pop_from_loadout(int id);

	// map helpers
	dir::LR entered_from() const;

	// for debug mode
	std::string print_direction(bool lr);

	// components
	PlayerController controller;
	Transponder transponder{};
	shape::Collider collider{};
	PlayerAnimation animation{};
	entity::Health health{};
	Indicator health_indicator;
	Indicator orb_indicator;

	// weapons
	std::optional<arms::Arsenal> arsenal{};

	sf::Vector2<float> apparent_position{};
	sf::Vector2<float> anchor_point{};
	sf::Vector2<float> hand_position{};
	sf::Vector2<float> sprite_offset{};
	sf::Vector2<float> sprite_dimensions{};
	sf::Vector2<float> sprite_position{};

	std::vector<vfx::Gravitator> antennae{};
	sf::Vector2<float> antenna_offset{4.f, -13.f};

	PlayerStats player_stats{0, 99999, 0.06f};
	PhysicsStats physics_stats{};
	PlayerFlags flags{};
	util::Cooldown hurt_cooldown{}; //for animation
	util::Cooldown force_cooldown{}; //for player hurt forces
	Counters counters{};
	std::vector<sf::Vector2<float>> accumulated_forces{};
	sf::Vector2<float> forced_momentum{};

	automa::ServiceProvider* m_services;

	// sprites
	sf::Sprite sprite{};
	flfx::SpriteHistory sprite_history{};

	// texture updater
	flfx::TextureUpdater texture_updater{};

	bool grav = true;

	bool start_cooldown{};
	bool sprite_flip{};

	int ledge_height{}; // temp for testing

	Catalog catalog{};

  private:
	struct {
		float stop{5.8f};
		float wallslide{-1.5f};
		float suspend{4.4f};
		float landed{0.4f};
		float run{0.02f};
	} thresholds{};
};

} // namespace player
