
#pragma once

#include <fornani/audio/Ambience.hpp>
#include <fornani/entities/atmosphere/Atmosphere.hpp>
#include <fornani/entities/enemy/EnemyCatalog.hpp>
#include <fornani/entities/item/Loot.hpp>
#include <fornani/entities/npc/NPC.hpp>
#include <fornani/entities/world/Animator.hpp>
#include <fornani/entities/world/Bed.hpp>
#include <fornani/entities/world/Chest.hpp>
#include <fornani/entities/world/Fire.hpp>
#include <fornani/entities/world/Inspectable.hpp>
#include <fornani/entity/EntitySet.hpp>
#include <fornani/graphics/Background.hpp>
#include <fornani/graphics/CameraController.hpp>
#include <fornani/graphics/DayNightShifter.hpp>
#include <fornani/graphics/Rain.hpp>
#include <fornani/graphics/Scenery.hpp>
#include <fornani/graphics/Transition.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/particle/Effect.hpp>
#include <fornani/particle/Emitter.hpp>
#include <fornani/shader/LightShader.hpp>
#include <fornani/shader/Palette.hpp>
#include <fornani/story/CutsceneCatalog.hpp>
#include <fornani/utils/CircleCollider.hpp>
#include <fornani/utils/Constants.hpp>
#include <fornani/utils/Shape.hpp>
#include <fornani/utils/Stopwatch.hpp>
#include <fornani/weapon/Grenade.hpp>
#include <fornani/weapon/Projectile.hpp>
#include <fornani/world/Breakable.hpp>
#include <fornani/world/Checkpoint.hpp>
#include <fornani/world/Destructible.hpp>
#include <fornani/world/Layer.hpp>
#include <fornani/world/Platform.hpp>
#include <fornani/world/Pushable.hpp>
#include <fornani/world/Spawner.hpp>
#include <fornani/world/Spike.hpp>
#include <fornani/world/SwitchBlock.hpp>
#include <fornani/world/TimerBlock.hpp>
#include <list>
#include <optional>
#include <vector>

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

enum class LevelState : std::uint8_t { game_over, camera_shake, spawn_enemy, transitioning };
enum class MapState : std::uint8_t { unobscure };
enum class LayerProperties : std::uint8_t { has_obscuring_layer, has_reverse_obscuring_layer };
enum class MapProperties : std::uint8_t { minimap, environmental_randomness, day_night_shift, timer, lighting };

struct EnemySpawn {
	sf::Vector2f pos{};
	int id{};
	int variant{};
};

struct MapAttributes {
	MapAttributes() = default;
	MapAttributes(dj::Json const& in);
	util::BitFlags<MapProperties> properties{};
	std::string ambience{};
	std::string music{};
	std::vector<int> atmosphere{};
	int style_id{};
	int special_drop_id{};
	int background_id{};
	sf::Color border_color{};

	void serialize(dj::Json& out);

  private:
	io::Logger m_logger{"Map"};
};

class Map {

  public:
	Map(automa::ServiceProvider& svc, player::Player& player);

	// methods
	void load(automa::ServiceProvider& svc, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, int room_number);
	void unserialize(automa::ServiceProvider& svc, int room_number, bool live = false);
	void update(automa::ServiceProvider& svc, std::optional<std::unique_ptr<gui::Console>>& console);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, std::optional<LightShader>& shader, sf::Vector2f cam);
	void render_background(automa::ServiceProvider& svc, sf::RenderWindow& win, std::optional<LightShader>& shader, sf::Vector2f cam);
	bool handle_entry(player::Player& player, util::Cooldown& enter_room);
	void spawn_projectile_at(automa::ServiceProvider& svc, arms::Weapon& weapon, sf::Vector2f pos, sf::Vector2f target = {}, float speed_multiplier = 1.f);
	void spawn_enemy(int id, sf::Vector2f pos, int variant = 0);
	void reveal_npc(std::string_view label);
	void manage_projectiles(automa::ServiceProvider& svc);
	void generate_collidable_layer(bool live = false);
	void generate_layer_textures(automa::ServiceProvider& svc) const;
	bool check_cell_collision(shape::Collider& collider, bool foreground = false);
	bool check_cell_collision_circle(shape::CircleCollider& collider, bool collide_with_platforms = true);
	sf::Vector2i get_circle_collision_result(shape::CircleCollider& collider, bool collide_with_platforms = true);
	void handle_cell_collision(shape::CircleCollider& collider);
	void handle_breakable_collision(shape::CircleCollider& collider);
	void clear_projectiles();
	void shake_camera();
	void clear();
	void wrap(sf::Vector2f& position) const;
	std::vector<std::unique_ptr<world::Layer>>& get_layers();
	std::unique_ptr<world::Layer>& get_middleground();
	std::unique_ptr<world::Layer>& get_obscuring_layer();
	sf::Vector2f get_nearest_target_point(sf::Vector2f from);
	sf::Vector2f last_checkpoint();

	void debug();

	bool nearby(shape::Shape& first, shape::Shape& second) const;
	bool within_bounds(sf::Vector2f test) const;
	bool overlaps_middleground(shape::Shape& test);

	[[nodiscard]] auto get_style_id() const -> int { return m_attributes.style_id; }
	[[nodiscard]] auto get_special_drop_id() const -> int { return m_attributes.special_drop_id; }
	[[nodiscard]] auto get_chunk_id_from_position(sf::Vector2f pos) const -> std::uint8_t;
	[[nodiscard]] auto get_chunk_dimensions() const -> sf::Vector2u { return dimensions / constants::u32_chunk_size; };
	[[nodiscard]] auto off_the_bottom(sf::Vector2f point) const -> bool { return point.y > real_dimensions.y + abyss_distance; }
	[[nodiscard]] auto camera_shake() const -> bool { return flags.state.test(LevelState::camera_shake); }
	[[nodiscard]] auto get_echo_count() const -> int { return sound.echo_count; }
	[[nodiscard]] auto get_echo_rate() const -> int { return sound.echo_rate; }
	[[nodiscard]] auto is_minimap() const -> bool { return m_attributes.properties.test(MapProperties::minimap); }
	[[nodiscard]] auto has_obscuring_layer() const -> bool { return m_layer_properties.test(LayerProperties::has_obscuring_layer); }
	[[nodiscard]] auto has_reverse_obscuring_layer() const -> bool { return m_layer_properties.test(LayerProperties::has_reverse_obscuring_layer); }
	[[nodiscard]] auto get_biome_string() const -> std::string { return m_metadata.biome; }
	[[nodiscard]] auto get_room_string() const -> std::string { return m_metadata.room; }
	[[nodiscard]] auto get_player_start() const -> sf::Vector2f { return m_player_start; }
	[[nodiscard]] auto has_entities() const -> bool { return m_entities.has_value(); }
	[[nodiscard]] auto enemies_cleared() const -> bool { return enemy_catalog.enemies.empty() && cooldowns.loading.is_complete(); }

	dj::Json const& get_json_data(automa::ServiceProvider& svc) const;

	std::size_t get_index_at_position(sf::Vector2f position);
	int get_tile_value_at_position(sf::Vector2f position);
	Tile& get_cell_at_position(sf::Vector2f position);

	template <typename T>
	std::vector<T*> get_entities() {
		std::vector<T*> ret;
		for (auto const& entity : m_entities.value().variables.entities) {
			if (auto* portal = dynamic_cast<T*>(entity.get())) { ret.push_back(portal); }
		}
		return ret;
	}

	// layers
	sf::Vector2<int> metagrid_coordinates{};
	sf::Vector2f real_dimensions{}; // pixel dimensions (maybe useless)
	sf::Vector2u dimensions{};		// points on the 32x32-unit grid

	// entities
	std::vector<arms::Projectile> active_projectiles{};
	std::vector<vfx::Emitter> active_emitters{};
	// std::vector<entity::Portal> portals{};
	std::vector<entity::Inspectable> inspectables{};
	std::vector<entity::Bed> beds{};
	std::vector<entity::Animator> animators{};
	std::vector<entity::Effect> effects{};
	std::array<std::vector<std::unique_ptr<vfx::Scenery>>, 6> scenery_layers{};
	std::vector<item::Loot> active_loot{};
	std::vector<entity::Chest> chests{};
	std::vector<std::unique_ptr<npc::NPC>> npcs{};
	std::vector<Platform> platforms{};

	std::list<Breakable> breakables{};
	std::vector<std::vector<std::list<Breakable>::iterator>> breakable_iterators{};
	std::vector<Pushable> pushables{};

	std::vector<Spawner> spawners{};
	std::vector<Spike> spikes{};
	std::vector<std::unique_ptr<SwitchButton>> switch_buttons{};
	std::vector<SwitchBlock> switch_blocks{};
	std::vector<Destructible> destructibles{};
	std::vector<Checkpoint> checkpoints{};
	std::vector<TimerBlock> timer_blocks{};
	std::vector<EnemySpawn> enemy_spawns{};
	std::vector<vfx::Atmosphere> atmosphere{};
	std::vector<sf::Vector2f> target_points{};
	std::vector<sf::Vector2f> home_points{};

	// vfx
	std::optional<vfx::Rain> rain{};
	std::optional<std::vector<Fire>> fire{};

	std::unique_ptr<graphics::Background> background{};
	graphics::Transition transition;

	enemy::EnemyCatalog enemy_catalog;
	CutsceneCatalog cutscene_catalog;

	sf::RectangleShape center_box{};

	std::string style_label{};

	struct {
		int breakables{};
		int pushables{};
	} styles{};

	float collision_barrier{2.5f};

	int room_id{}; // should be assigned to its constituent chunks
	bool debug_mode{};
	bool game_over{};
	bool use_template{};

	player::Player* player;
	automa::ServiceProvider* m_services;

	util::Cooldown spawning{2};
	util::Counter spawn_counter{};
	struct {
		util::Cooldown fade_obscured{};
		util::Cooldown loading{};
	} cooldowns{};

	// debug
	util::Stopwatch stopwatch{};
	util::Cooldown end_demo{500};
	std::vector<PointLight> point_lights{};
	float darken_factor{};

  private:
	MapAttributes m_attributes{};
	util::BitFlags<LayerProperties> m_layer_properties{};
	std::optional<EntitySet> m_entities{};

	std::optional<Palette> m_palette{};
	int abyss_distance{512};
	sf::Vector2f m_player_start{};

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
		int echo_rate{};
		int echo_count{};
	} sound{};
	struct {
		util::BitFlags<LevelState> state{};
		util::BitFlags<MapState> map_state{};
	} flags{};
	int m_middleground{};

	io::Logger m_logger{"Map"};
};

} // namespace fornani::world
