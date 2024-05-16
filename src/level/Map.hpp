
#pragma once

#include <string>
#include <vector>
#include "../entities/enemy/EnemyCatalog.hpp"
#include "../entities/world/Animator.hpp"
#include "../entities/world/Inspectable.hpp"
#include "../entities/world/Portal.hpp"
#include "../entities/world/SavePoint.hpp"
#include "../graphics/Background.hpp"
#include "../graphics/Transition.hpp"
#include "../setup/MapLookups.hpp"
#include "Grid.hpp"
#include "../utils/Random.hpp"
#include "../utils/Shape.hpp"
#include "../particle/Effect.hpp"
#include "../weapon/Projectile.hpp"
#include "../entities/item/Loot.hpp"
#include "../entities/world/Chest.hpp"
#include "../entities/npc/NPC.hpp"
#include "Platform.hpp"
#include "../weapon/Grenade.hpp"

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

enum LAYER_ORDER {
	BACKGROUND = 0,
	MIDDLEGROUND = 4,
	FOREGROUND = 7,
};

// a Layer is a grid with a render priority and a flag to determine if scene entities can collide with it.
// for for loop, the current convention is that the only collidable layer is layer 4 (index 3), or the middleground.

class Layer {

  public:
	Layer() = default;
	Layer(uint8_t o, bool c, sf::Vector2<uint32_t> dim) : render_order(o), collidable(c), dimensions(dim) { grid = Grid({dim.x, dim.y}); }
	Grid grid{};
	uint8_t render_order{};
	bool collidable{};
	sf::Vector2<uint32_t> dimensions{};
};

// a Map is just a set of layers that will render on top of each other

class Map {

  public:
	using Vec = sf::Vector2<float>;
	using Vecu16 = sf::Vector2<uint32_t>;

	Map() = default;
	Map(automa::ServiceProvider& svc, player::Player& player);

	// methods
	void load(automa::ServiceProvider& svc, std::string_view room);
	void update(automa::ServiceProvider& svc, gui::Console& console, gui::InventoryWindow& inventory_window);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void render_background(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void render_console(automa::ServiceProvider& svc, gui::Console& console, sf::RenderWindow& win);
	Tile& tile_at(const uint8_t i, const uint8_t j);
	shape::Shape& shape_at(const uint8_t i, const uint8_t j);
	void spawn_projectile_at(automa::ServiceProvider& svc, arms::Weapon& weapon, sf::Vector2<float> pos);
	void manage_projectiles(automa::ServiceProvider& svc);
	void generate_collidable_layer();
	void generate_layer_textures(automa::ServiceProvider& svc);
	bool check_cell_collision(shape::Collider collider);
	void handle_grappling_hook(automa::ServiceProvider& svc, arms::Projectile& proj);
	Vec get_spawn_position(int portal_source_map_id);

	bool nearby(shape::Shape& first, shape::Shape& second) const;
	[[nodiscard]] auto off_the_bottom(sf::Vector2<float> point) const -> bool { return point.y > real_dimensions.y + abyss_distance; }

	// layers
	std::vector<Layer> layers;
	std::vector<uint32_t> collidable_indeces{};		// generated on load to reduce collision checks in hot code
	Vec real_dimensions{};		// pixel dimensions (maybe useless)
	Vecu16 dimensions{};		// points on the 32x32-unit grid
	Vecu16 chunk_dimensions{};	// how many chunks (16x16 squares) in the room

	// json for data loading
	dj::Json metadata{};
	dj::Json tiles{};
	dj::Json inspectable_data{};

	// entities
	std::vector<arms::Projectile> active_projectiles{};
	std::vector<arms::Grenade> active_grenades{};
	std::vector<vfx::Emitter> active_emitters{};
	std::vector<entity::Portal> portals{};
	std::vector<entity::Inspectable> inspectables{};
	std::vector<entity::Animator> animators{};
	std::vector<entity::Effect> effects{};
	std::vector<item::Loot> active_loot{};
	std::vector<entity::Chest> chests{};
	std::vector<npc::NPC> npcs{};
	std::vector<Platform> platforms{};
	entity::SavePoint save_point;

	std::unique_ptr<bg::Background> background{};
	flfx::Transition transition;

	enemy::EnemyCatalog enemy_catalog;

	// minimap
	sf::View minimap{};
	sf::RectangleShape minimap_tile{};
	sf::RectangleShape borderbox{};

	//layers
	std::array<sf::RenderTexture, NUM_LAYERS> layer_textures{};
	sf::Sprite tile_sprite{};
	sf::Sprite layer_sprite{};
	std::string_view style_label{};
	int style_id{};
	int native_style_id{};

	float collision_barrier{2.5f};

	int room_id{}; // should be assigned to its constituent chunks
	bool game_over{false};
	bool show_minimap{false};
	bool debug_mode{false};

	player::Player* player;

	util::Cooldown loading{}; // shouldn't exist

	private:
	int abyss_distance{400};
};

} // namespace world
