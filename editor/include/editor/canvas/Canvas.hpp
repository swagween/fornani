
#pragma once

#include <SFML/Graphics.hpp>
#include <stdio.h>
#include <string>
#include "Layer.hpp"
#include <fstream>
#include <vector>
#include <cstdio>
#include <filesystem>
#include <system_error>
#include <iostream>
#include <djson/json.hpp>
#include "editor/util/BitFlags.hpp"
#include "editor/util/Camera.hpp"
#include "EntitySet.hpp"
#include "Background.hpp"

namespace data {
class ResourceFinder;
}

namespace pi {

int const NUM_LAYERS{8};
uint32_t const chunk_size{16};

enum class Backdrop {
	BG_DUSK,
	BG_SUNRISE,
	BG_OPEN_SKY,
	BG_ROSY_HAZE,
	BG_NIGHT,
	BG_DAWN,
	BG_OVERCAST,

	BG_SLIME,
	BG_BLACK,
	BG_NAVY,
	BG_DIRT,
	BG_GEAR,
	BG_LIBRARY,
	BG_GRANITE,
	BG_RUINS,
	BG_CREVASSE,
	BG_DEEP,
	BG_GROVE,

	END
};

enum LAYER_ORDER {
	BACKGROUND = 0,
	MIDDLEGROUND = 4,
	FOREGROUND = 7,
};

enum class Style { firstwind, overturned, base, factory, greatwing, provisional, END };

enum class CanvasProperties { editable };
enum class CanvasState { hovered };
enum class SelectionType { neutral, palette, canvas };

struct Map {
	std::vector<Layer> layers{};
};

class Tool;

class Canvas {

  public:
	Canvas(SelectionType type);
	Canvas(sf::Vector2<uint32_t> dim, SelectionType type);
	void update(Tool& tool, bool transformed = false);
	void render(sf::RenderWindow& win, sf::Sprite& tileset);
	void load(data::ResourceFinder& finder, std::string const& room_name, bool local = false);
	bool save(data::ResourceFinder& finder, std::string const& room_name);
	void clear();
	void save_state(Tool& tool, bool force = false);
	void undo();
	void redo();
	void clear_redo_states();
	void unhover();
	void move(sf::Vector2<float> distance);
	void set_position(sf::Vector2<float> to_position);
	void set_origin(sf::Vector2<float> to_origin);
	void set_offset_from_center(sf::Vector2<float> offset);
	void set_scale(float to_scale);
	void resize(sf::Vector2i adjustment);
	void center(sf::Vector2<float> point);
	void zoom(float amount);
	void set_backdrop_color(sf::Color color);
	Map& get_layers();
	sf::Vector2<int> get_tile_coord(int lookup);
	[[nodiscard]] auto get_selection_type() const -> SelectionType { return type; }
	[[nodiscard]] auto states_empty() const -> bool { return map_states.empty(); }
	[[nodiscard]] auto hovered() const -> bool { return state.test(CanvasState::hovered); }
	[[nodiscard]] auto editable() const -> bool { return properties.test(CanvasProperties::editable); }
	[[nodiscard]] auto chunk_dimensions() const -> sf::Vector2<uint32_t> { return dimensions / chunk_size; }
	[[nodiscard]] auto get_position() const -> sf::Vector2<float> { return camera.position; }
	[[nodiscard]] auto get_scaled_position() const -> sf::Vector2<float> { return camera.position / scale; }
	[[nodiscard]] auto get_real_dimensions() const -> sf::Vector2<float> { return real_dimensions * scale; }
	[[nodiscard]] auto get_center() const -> sf::Vector2<float> { return real_dimensions * 0.5f; }
	[[nodiscard]] auto get_origin() const -> sf::Vector2<float> { return origin; }
	[[nodiscard]] auto get_offset_from_center() const -> sf::Vector2<float> { return offset_from_center; }
	[[nodiscard]] auto get_scaled_center() const -> sf::Vector2<float> { return real_dimensions * 0.5f * scale; }
	[[nodiscard]] auto i_native_chunk_size() const -> int { return 16; }
	[[nodiscard]] auto u_cell_size() const -> int { return static_cast<uint32_t>(i_cell_size()); }
	[[nodiscard]] auto i_cell_size() const -> int { return static_cast<int>(f_cell_size()); }
	[[nodiscard]] auto f_cell_size() const -> float { return 32.f * scale; }
	[[nodiscard]] auto f_chunk_size() const -> float { return 16.f * scale; }
	[[nodiscard]] auto f_native_cell_size() const -> float { return 32.f; }
	[[nodiscard]] auto f_native_chunk_size() const -> float { return 16.f; }
	[[nodiscard]] auto get_scale() const -> float { return scale; }
	[[nodiscard]] auto get_i_style() const -> int { return static_cast<int>(styles.tile); }
	[[nodiscard]] auto within_zoom_limits(float delta) const -> bool { return get_scale() + delta >= min_scale && get_scale() + delta <= max_scale; }
	[[nodiscard]] auto within_bounds(sf::Vector2<float> const& point) const -> bool {
		return point.x > camera.position.x && point.x < real_dimensions.x + camera.position.x && point.y > camera.position.y && point.y < real_dimensions.y + camera.position.y;
	}
	[[nodiscard]] auto undo_states_size() const -> std::size_t { return map_states.size(); }
	[[nodiscard]] auto redo_states_size() const -> std::size_t { return redo_states.size(); }

	void replace_tile(uint32_t from, uint32_t to, int layer_index);
	void edit_tile_at(int i, int j, int new_val, int layer_index);
	void erase_at(int i, int j, int layer_index);
	int tile_val_at(int i, int j, int layer);
	int tile_val_at_scaled(int i, int j, int layer);
	sf::Vector2<float> get_tile_position_at(int i, int j, int layer = 4);
	Tile& get_tile_at(int i, int j, int layer = 4);
	TILE_TYPE lookup_type(int idx);

	// layers
	sf::Vector2<uint32_t> dimensions{};
	sf::Vector2<int> metagrid_coordinates{};

	struct {
		bool show_grid{true};
		bool show_all_layers{true};
		bool show_obscured_layer{true};
		bool show_indicated_layers{true};
		bool show_entities{true};
		bool show_background{};
	} flags{};

	EntitySet entities;

	// read and write
	struct {
		dj::Json meta{};
		dj::Json tiles{};
	} data{};

	struct {
		float cell_size{32.f};
	} constants{};

	struct {
		Style tile{};
		int breakable{};
	} styles{};

	struct {
		bool flag{};
		int type{};
		int id{};
		int source{};
	} cutscene{};

	sf::Vector2u player_start{};
	int active_layer{};

	Backdrop bg{};

	uint32_t room_id{}; // should be assigned to its constituent chunks
  private:
	sf::Vector2<float> origin{};
	sf::Vector2<float> real_dimensions{};
	sf::Vector2<float> offset_from_center{};
	std::vector<Map> map_states{};
	std::vector<Map> redo_states{};
	util::BitFlags<CanvasState> state{};
	util::BitFlags<CanvasProperties> properties{};
	Camera camera;
	sf::RectangleShape box{};
	sf::RectangleShape gridbox{};
	sf::RectangleShape chunkbox{};
	sf::RectangleShape border{};
	std::unique_ptr<Background> background{};

	SelectionType type{};

	float scale{1.f};
	float min_scale{0.1f};
	float max_scale{4.f};
};

} // namespace pi
