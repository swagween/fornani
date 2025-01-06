
#pragma once

#include <string>
#include <vector>
#include "../entities/enemy/EnemyCatalog.hpp"
#include "../entities/world/Animator.hpp"
#include "../entities/world/Inspectable.hpp"
#include "../entities/world/Portal.hpp"
#include "../entities/world/SavePoint.hpp"
#include "../entities/world/Vine.hpp"
#include "../entities/world/Grass.hpp"
#include "../entities/world/Fire.hpp"
#include "../graphics/Background.hpp"
#include "../graphics/Scenery.hpp"
#include "../graphics/Transition.hpp"
#include "../graphics/Rain.hpp"
#include "Grid.hpp"
#include "../utils/Random.hpp"
#include "../utils/Shape.hpp"
#include "../particle/Effect.hpp"
#include "../weapon/Projectile.hpp"
#include "../entities/item/Loot.hpp"
#include "../entities/world/Chest.hpp"
#include "../entities/npc/NPC.hpp"
#include "../entities/world/Bed.hpp"
#include "Platform.hpp"
#include "Breakable.hpp"
#include "Pushable.hpp"
#include "Spawner.hpp"
#include "Spike.hpp"
#include "SwitchBlock.hpp"
#include "Destroyable.hpp"
#include "Checkpoint.hpp"
#include "../weapon/Grenade.hpp"
#include "../story/CutsceneCatalog.hpp"
#include "../utils/Stopwatch.hpp"
#include "../utils/CircleCollider.hpp"
#include "../audio/Ambience.hpp"
#include "../entities/atmosphere/Atmosphere.hpp"

int const NUM_LAYERS{8};
int const CHUNK_SIZE{16};
int const CELL_SIZE{32};

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace gui {
class Console;
class Portrait;
class InventoryWindow;
}

namespace world {

enum class LevelState { game_over, camera_shake, spawn_enemy };
enum class MapState { unobscure };

// a Layer is a grid with a render priority and a flag to determine if scene entities can collide with it.
// for for loop, the current convention is that the only collidable layer is layer 4 (index 3), or the middleground.

class Layer {
  public:
	Layer() = default;
	Layer(uint8_t o, bool c, sf::Vector2<uint32_t> dim, dj::Json& source) : render_order(o), collidable(c), dimensions(dim), grid(dim, source) {}
	[[nodiscard]] auto background() const -> bool { return render_order < 4; }
	[[nodiscard]] auto foreground() const -> bool { return render_order > 3; }
	[[nodiscard]] auto middleground() const -> bool { return render_order == 4; }
	[[nodiscard]] auto obscuring() const -> bool { return render_order == 7; }
	[[nodiscard]] auto get_render_order() const -> uint8_t { return render_order; }
	Grid grid;
	bool collidable{};
	sf::Vector2<uint32_t> dimensions{};

  private:
	uint8_t render_order{};
};

struct EnemySpawn {
	sf::Vector2<float> pos{};
	int id{};
};

// a Map is just a set of layers that will render on top of each other

class Map {

  public:
	using Vec = sf::Vector2<float>;
	using Vecu16 = sf::Vector2<uint32_t>;

	Map() = default;
	Map(automa::ServiceProvider& svc, player::Player& player, gui::Console& console);
	~Map() {}

	// methods
	void load(automa::ServiceProvider& svc, int room_number, bool soft = false);
	void update(automa::ServiceProvider& svc, gui::Console& console, gui::InventoryWindow& inventory_window);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void render_background(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void render_console(automa::ServiceProvider& svc, gui::Console& console, sf::RenderWindow& win);
	void spawn_projectile_at(automa::ServiceProvider& svc, arms::Weapon& weapon, sf::Vector2<float> pos, sf::Vector2<float> target = {});
	void spawn_enemy(int id, sf::Vector2<float> pos);
	void manage_projectiles(automa::ServiceProvider& svc);
	void generate_collidable_layer(bool live = false);
	void generate_layer_textures(automa::ServiceProvider& svc);
	bool check_cell_collision(shape::Collider& collider, bool foreground = false);
	bool check_cell_collision_circle(shape::CircleCollider& collider, bool collide_with_platforms = true);
	void handle_cell_collision(shape::CircleCollider& collider);
	void shake_camera();
	void clear();
	void wrap(sf::Vector2<float>& position) const;
	std::vector<Layer>& get_layers();
	Layer& get_middleground();
	npc::NPC& get_npc(int id);
	Vec get_spawn_position(int portal_source_map_id);
	sf::Vector2<float> get_nearest_target_point(sf::Vector2<float> from);
	sf::Vector2<float> last_checkpoint();

	void debug();

	bool nearby(shape::Shape& first, shape::Shape& second) const;
	bool within_bounds(sf::Vector2<float> test) const;
	bool overlaps_middleground(shape::Shape& test);
	[[nodiscard]] auto off_the_bottom(sf::Vector2<float> point) const -> bool { return point.y > real_dimensions.y + abyss_distance; }
	[[nodiscard]] auto camera_shake() const -> bool { return flags.state.test(LevelState::camera_shake); }
	std::size_t get_index_at_position(sf::Vector2<float> position);
	int get_tile_value_at_position(sf::Vector2<float> position);
	Tile& get_cell_at_position(sf::Vector2<float> position);

	// layers
	sf::Vector2<int> metagrid_coordinates{};
	// std::vector<Layer> layers{};
	Vec real_dimensions{};	   // pixel dimensions (maybe useless)
	Vecu16 dimensions{};	   // points on the 32x32-unit grid
	Vecu16 chunk_dimensions{}; // how many chunks (16x16 squares) in the room

	dj::Json inspectable_data{};

	// entities
	std::vector<arms::Projectile> active_projectiles{};
	std::vector<arms::Grenade> active_grenades{};
	std::vector<vfx::Emitter> active_emitters{};
	std::vector<entity::Portal> portals{};
	std::vector<entity::Inspectable> inspectables{};
	std::vector<entity::Bed> beds{};
	std::vector<entity::Animator> animators{};
	std::vector<entity::Effect> effects{};
	std::vector<std::unique_ptr<entity::Vine>> vines{};
	std::vector<std::unique_ptr<entity::Grass>> grass{};
	std::array<std::vector<std::unique_ptr<vfx::Scenery>>, 6> scenery_layers{};
	std::vector<item::Loot> active_loot{};
	std::vector<entity::Chest> chests{};
	std::vector<npc::NPC> npcs{};
	std::vector<Platform> platforms{};
	std::vector<Breakable> breakables{};
	std::vector<Pushable> pushables{};
	std::vector<Spawner> spawners{};
	std::vector<Spike> spikes{};
	std::vector<std::unique_ptr<SwitchButton>> switch_buttons{};
	std::vector<SwitchBlock> switch_blocks{};
	std::vector<Destroyable> destroyers{};
	std::vector<Checkpoint> checkpoints{};
	std::vector<EnemySpawn> enemy_spawns{};
	entity::SavePoint save_point;
	std::vector<vfx::Atmosphere> atmosphere{};
	std::vector<sf::Vector2<float>> target_points{};

	// vfx
	std::optional<vfx::Rain> rain{};
	std::optional<std::vector<Fire>> fire{};

	// sfx
	audio::Ambience ambience{};

	std::unique_ptr<bg::Background> background{};
	flfx::Transition transition;
	flfx::Transition soft_reset;

	enemy::EnemyCatalog enemy_catalog;
	fornani::CutsceneCatalog cutscene_catalog;

	sf::RectangleShape tile{};
	sf::RectangleShape borderbox{};
	sf::RectangleShape center_box{};

	// layers
	struct {
		sf::RenderTexture foreground{};
		sf::RenderTexture background{};
		sf::RenderTexture obscuring{};
		sf::RenderTexture reverse{};
	} textures{};
	std::string_view style_label{};

	int room_lookup{};
	int style_id{};
	int native_style_id{};
	struct {
		int breakables{};
		int pushables{};
	} styles{};

	float collision_barrier{2.5f};

	int room_id{}; // should be assigned to its constituent chunks
	bool game_over{false};
	bool show_minimap{false};
	bool debug_mode{false};

	player::Player* player;
	automa::ServiceProvider* m_services;
	gui::Console* m_console;

	util::Cooldown loading{}; // shouldn't exist
	util::Cooldown spawning{2};
	util::Counter spawn_counter{};
	struct {
		util::Cooldown fade_obscured{128};
	} cooldowns{};

	// debug
	util::Stopwatch stopwatch{};
	util::Cooldown end_demo{500};

  private:
	int abyss_distance{400};
	struct {
		util::BitFlags<LevelState> state{};
		util::BitFlags<MapState> map_state{};
	} flags{};
};

} // namespace world
