
#pragma once

#include "Breakable.hpp"
#include "Checkpoint.hpp"
#include "Destroyable.hpp"
#include "Grid.hpp"
#include "Platform.hpp"
#include "Pushable.hpp"
#include "Spawner.hpp"
#include "Spike.hpp"
#include "SwitchBlock.hpp"
#include "fornani/audio/Ambience.hpp"
#include "fornani/entities/atmosphere/Atmosphere.hpp"
#include "fornani/entities/enemy/EnemyCatalog.hpp"
#include "fornani/entities/item/Loot.hpp"
#include "fornani/entities/npc/NPC.hpp"
#include "fornani/entities/world/Animator.hpp"
#include "fornani/entities/world/Bed.hpp"
#include "fornani/entities/world/Chest.hpp"
#include "fornani/entities/world/Fire.hpp"
#include "fornani/entities/world/Grass.hpp"
#include "fornani/entities/world/Inspectable.hpp"
#include "fornani/entities/world/Portal.hpp"
#include "fornani/entities/world/SavePoint.hpp"
#include "fornani/entities/world/Vine.hpp"
#include "fornani/graphics/Background.hpp"
#include "fornani/graphics/CameraController.hpp"
#include "fornani/graphics/DayNightShifter.hpp"
#include "fornani/graphics/Rain.hpp"
#include "fornani/graphics/Scenery.hpp"
#include "fornani/graphics/Transition.hpp"
#include "fornani/particle/Effect.hpp"
#include "fornani/story/CutsceneCatalog.hpp"
#include "fornani/utils/CircleCollider.hpp"
#include "fornani/utils/Shape.hpp"
#include "fornani/utils/Stopwatch.hpp"
#include "fornani/weapon/Grenade.hpp"
#include "fornani/weapon/Projectile.hpp"

#include <optional>
#include <vector>

#include "fornani/particle/Emitter.hpp"

constexpr unsigned int u_chunk_size_v{16u};

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::gui {
class Console;
class Portrait;
class InventoryWindow;
} // namespace fornani::gui

namespace fornani::world {

enum class LevelState : uint8_t { game_over, camera_shake, spawn_enemy };
enum class MapState : uint8_t { unobscure };
enum class MapProperties : uint8_t { minimap, has_obscuring_layer, has_reverse_obscuring_layer };
enum class LayerType : uint8_t { background, middleground, foreground, reverse_obscuring, obscuring };

struct LayerTexture {
	sf::RenderTexture day{};
	sf::RenderTexture twilight{};
	sf::RenderTexture night{};
};

class Layer {
  public:
	Layer() = default;
	Layer(uint8_t o, sf::Vector2i partition, sf::Vector2<uint32_t> dim, dj::Json& source, float spacing, bool has_obscuring, bool has_reverse_obscuring)
		: render_order(o), collidable(o == partition.x), dimensions(dim), grid(dim, source, spacing) {
		auto order = static_cast<int>(o);
		if (order < partition.x) { type = LayerType::background; }
		if (order == partition.x) { type = LayerType::middleground; }
		if (order > partition.x) { type = LayerType::foreground; }
		if (order == partition.y - 2 && has_reverse_obscuring) { type = LayerType::reverse_obscuring; }
		if (order == partition.y - 1 && has_obscuring) { type = LayerType::obscuring; }
	}
	[[nodiscard]] auto background() const -> bool { return type == LayerType::background; }
	[[nodiscard]] auto foreground() const -> bool { return type == LayerType::foreground; }
	[[nodiscard]] auto middleground() const -> bool { return type == LayerType::middleground; }
	[[nodiscard]] auto obscuring() const -> bool { return type == LayerType::obscuring; }
	[[nodiscard]] auto reverse_obscuring() const -> bool { return type == LayerType::reverse_obscuring; }
	[[nodiscard]] auto get_render_order() const -> uint8_t { return render_order; }
	[[nodiscard]] auto get_i_render_order() const -> int { return static_cast<int>(render_order); }
	[[nodiscard]] auto get_layer_type() const -> LayerType { return type; }
	Grid grid;
	bool collidable{};
	sf::Vector2<uint32_t> dimensions{};

  private:
	uint8_t render_order{};
	LayerType type{};
};

struct EnemySpawn {
	sf::Vector2<float> pos{};
	int id{};
};

class Map {

  public:
	using Vec = sf::Vector2<float>;

	Map(automa::ServiceProvider& svc, player::Player& player, gui::Console& console);
	~Map() {}

	// methods
	void load(automa::ServiceProvider& svc, int room_number, bool soft = false);
	void update(automa::ServiceProvider& svc, gui::Console& console);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void render_background(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
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
	Layer& get_obscuring_layer();
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
	[[nodiscard]] auto get_echo_count() const -> int { return sound.echo_count; }
	[[nodiscard]] auto get_echo_rate() const -> int { return sound.echo_rate; }
	[[nodiscard]] auto chunk_dimensions() const -> sf::Vector2u { return dimensions / u_chunk_size_v; }
	[[nodiscard]] auto is_minimap() const -> bool { return flags.properties.test(MapProperties::minimap); }
	[[nodiscard]] auto has_obscuring_layer() const -> bool { return flags.properties.test(MapProperties::has_obscuring_layer); }
	[[nodiscard]] auto has_reverse_obscuring_layer() const -> bool { return flags.properties.test(MapProperties::has_reverse_obscuring_layer); }
	[[nodiscard]] auto get_biome_string() const -> std::string { return m_metadata.biome; }
	[[nodiscard]] auto get_room_string() const -> std::string { return m_metadata.room; }
	std::size_t get_index_at_position(sf::Vector2<float> position);
	int get_tile_value_at_position(sf::Vector2<float> position);
	Tile& get_cell_at_position(sf::Vector2<float> position);

	// layers
	sf::Vector2<int> metagrid_coordinates{};
	Vec real_dimensions{};	   // pixel dimensions (maybe useless)
	sf::Vector2u dimensions{}; // points on the 32x32-unit grid

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
	CutsceneCatalog cutscene_catalog;

	sf::RectangleShape borderbox{};
	sf::RectangleShape center_box{};
	sf::Vector2f barrier;
	sf::Vector2f scaled_barrier;

	// layers
	struct {
		sf::RenderTexture greyblock{};
		LayerTexture foreground{};
		LayerTexture background{};
		std::optional<LayerTexture> obscuring{};
		std::optional<LayerTexture> reverse_obscuring{};
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

	util::Cooldown spawning{2};
	util::Counter spawn_counter{};
	struct {
		util::Cooldown fade_obscured{};
		util::Cooldown loading{};
	} cooldowns{};

	// debug
	util::Stopwatch stopwatch{};
	util::Cooldown end_demo{500};

  private:
	void draw_barrier(sf::RenderTexture& tex, sf::Sprite& tile, Tile& cell);
	int abyss_distance{400};
	struct {
		std::string biome{};
		std::string room{};
	} m_metadata{};
	struct {
		graphics::ShakeProperties shake_properties{};
		util::Cooldown cooldown{};
		graphics::DayNightShifter shifter{};
	} m_camera_effects{};
	struct {
		util::BitFlags<LevelState> state{};
		util::BitFlags<MapState> map_state{};
		util::BitFlags<MapProperties> properties{};
	} flags{};
	struct {
		int echo_rate{};
		int echo_count{};
	} sound{};
	int m_middleground{};
};

} // namespace fornani::world
