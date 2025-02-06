
#pragma once

#include "fornani/graphics/CameraController.hpp"
#include "editor/util/BitFlags.hpp"
#include "editor/util/SelectBox.hpp"
#include "EntitySet.hpp"
#include "Background.hpp"
#include "Map.hpp"

#include <string>
#include <vector>
#include <filesystem>
#include <iostream>

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>

namespace data {
class ResourceFinder;
}

namespace pi {

enum class StyleType { firstwind, overturned, base, factory, greatwing, END };

enum class CanvasProperties { editable };
enum class CanvasState { hovered };

struct Theme {
	std::string music{};
	std::string ambience{};
	std::vector<int> atmosphere{};
};

constexpr inline int chunk_size_v{16};
constexpr inline int default_num_layers_v{8};
constexpr inline int default_middleground_v{4};

class Style {
  public:
	Style(StyleType type) : type(type) {
		switch (type) {
		case StyleType::firstwind: label = "firstwind"; break;
		case StyleType::overturned: label = "overturned"; break;
		case StyleType::base: label = "base"; break;
		case StyleType::factory: label = "factory"; break;
		case StyleType::greatwing: label = "greatwing"; break;
		default: label = "<none>"; break;
		}
		label_c_str = label.c_str();
	}

	[[nodiscard]] auto get_label_char() -> const char*& { return label_c_str; };
	[[nodiscard]] auto get_label() const -> std::string { return label; };
	[[nodiscard]] auto get_type() const -> StyleType { return type; };
	[[nodiscard]] auto get_i_type() const -> int { return static_cast<int>(type); };

  private:
	StyleType type{};
	std::string label{};
	char const* label_c_str{};
};

class Tool;

class Canvas {

  public:
	Canvas(data::ResourceFinder& finder, SelectionType type, StyleType style = StyleType::firstwind, Backdrop backdrop = Backdrop::black, int num_layers = default_num_layers_v);
	Canvas(data::ResourceFinder& finder, sf::Vector2<uint32_t> dim, SelectionType type, StyleType style, Backdrop backdrop, int num_layers = default_num_layers_v);
	void update(Tool& tool);
	void render(sf::RenderWindow& win, sf::Sprite& tileset);
	void load(data::ResourceFinder& finder, std::string const& region, std::string const& room_name, bool local = false);
	bool save(data::ResourceFinder& finder, std::string const& region, std::string const& room_name);
	void clear();
	void save_state(Tool& tool, bool force = false);
	void undo();
	void redo();
	void clear_redo_states();
	void hover();
	void unhover();
	void move(sf::Vector2<float> distance);
	void set_position(sf::Vector2<float> to_position);
	void set_origin(sf::Vector2<float> to_origin);
	void set_offset_from_center(sf::Vector2<float> offset);
	void set_scale(float to_scale);
	void resize(sf::Vector2i adjustment);
	void center(sf::Vector2<float> point);
	void constrain(sf::Vector2<float> bounds);
	void zoom(float amount);
	void set_backdrop_color(sf::Color color);
	void set_grid_texture();
	void activate_middleground();
	Map& get_layers();
	sf::Vector2<int> get_tile_coord(int lookup);
	[[nodiscard]] auto get_selection_type() const -> SelectionType { return type; }
	[[nodiscard]] auto states_empty() const -> bool { return map_states.empty(); }
	[[nodiscard]] auto is_palette() const -> bool { return type == SelectionType::palette; }
	[[nodiscard]] auto hovered() const -> bool { return state.test(CanvasState::hovered); }
	[[nodiscard]] auto editable() const -> bool { return properties.test(CanvasProperties::editable); }
	[[nodiscard]] auto chunk_dimensions() const -> sf::Vector2<uint32_t> { return dimensions / u_native_chunk_size(); }
	[[nodiscard]] auto get_position() const -> sf::Vector2<float> { return position; }
	[[nodiscard]] auto get_scaled_position() const -> sf::Vector2<float> { return position / scale; }
	[[nodiscard]] auto get_real_dimensions() const -> sf::Vector2<float> { return real_dimensions * scale; }
	[[nodiscard]] auto get_center() const -> sf::Vector2<float> { return real_dimensions * 0.5f; }
	[[nodiscard]] auto get_origin() const -> sf::Vector2<float> { return origin; }
	[[nodiscard]] auto get_offset_from_center() const -> sf::Vector2<float> { return offset_from_center; }
	[[nodiscard]] auto get_scaled_center() const -> sf::Vector2<float> { return real_dimensions * 0.5f * scale; }
	[[nodiscard]] auto u_native_chunk_size() const -> std::uint32_t { return static_cast<std::uint32_t>(chunk_size_v); }
	[[nodiscard]] auto i_native_chunk_size() const -> int { return chunk_size_v; }
	[[nodiscard]] auto f_native_chunk_size() const -> float { return static_cast<float>(chunk_size_v); }
	[[nodiscard]] auto u_cell_size() const -> std::uint32_t { return static_cast<uint32_t>(i_cell_size()); }
	[[nodiscard]] auto i_cell_size() const -> int { return static_cast<int>(f_cell_size()); }
	[[nodiscard]] auto f_cell_size() const -> float { return f_native_cell_size() * scale; }
	[[nodiscard]] auto f_chunk_size() const -> float { return f_native_chunk_size() * scale; }
	[[nodiscard]] auto f_native_cell_size() const -> float { return 32.f; }
	[[nodiscard]] auto get_scale() const -> float { return scale; }
	[[nodiscard]] auto get_i_style() const -> int { return static_cast<int>(styles.tile.get_type()); }
	[[nodiscard]] auto within_zoom_limits(float delta) const -> bool { return get_scale() + delta >= min_scale && get_scale() + delta <= max_scale; }
	[[nodiscard]] auto within_bounds(sf::Vector2<float> const& point) const -> bool {
		return point.x > position.x && point.x < real_dimensions.x + position.x && point.y > position.y && point.y < real_dimensions.y + position.y;
	}
	[[nodiscard]] auto undo_states_size() const -> std::size_t { return map_states.size(); }
	[[nodiscard]] auto redo_states_size() const -> std::size_t { return redo_states.size(); }
	[[nodiscard]] auto middleground() const -> int { return map_states.back().get_middleground(); }
	[[nodiscard]] auto last_layer() const -> int { return static_cast<int>(map_states.back().layers.size() - 1); }

	void replace_tile(uint32_t from, uint32_t to, int layer_index);
	void edit_tile_at(int i, int j, int new_val, int layer_index);
	void erase_at(int i, int j, int layer_index);
	int tile_val_at(int i, int j, int layer);
	int tile_val_at_scaled(int i, int j, int layer);
	sf::Vector2<float> get_tile_position_at(int i, int j, int layer = 0);
	Tile& get_tile_at(int i, int j, int layer = 0);

	// layers
	sf::Vector2<uint32_t> dimensions{};
	sf::Vector2<int> metagrid_coordinates{};

	struct {
		bool show_grid{true};
		bool show_all_layers{true};
		bool show_current_layer{false};
		bool show_obscured_layer{true};
		bool show_indicated_layers{true};
		bool show_entities{true};
		bool show_background{};
	} flags{};

	EntitySet entities;

	struct {
		dj::Json meta{};
	} data{};

	struct {
		float cell_size{32.f};
	} constants{};

	struct {
		Style tile;
	} styles;

	Theme m_theme{};
	std::unique_ptr<Background> background{};

	struct {
		bool flag{};
		int type{};
		int id{};
		int source{};
	} cutscene{};

	struct {
		fornani::graphics::ShakeProperties shake_properties{};
		int frequency_in_seconds{};
	} m_camera_effects{};

	sf::Vector2u player_start{};
	int active_layer{};

	uint32_t room_id{};

	bool minimap{};

  private:
	sf::Vector2<float> position{};
	sf::RenderTexture grid_texture{};
	sf::Vector2<float> origin{};
	sf::Vector2<float> real_dimensions{};
	sf::Vector2<float> offset_from_center{};
	std::vector<Map> map_states{};
	std::vector<Map> redo_states{};
	util::BitFlags<CanvasState> state{};
	util::BitFlags<CanvasProperties> properties{};
	sf::RectangleShape box{};
	sf::RectangleShape gridbox{};
	sf::RectangleShape chunkbox{};
	sf::RectangleShape border{};

	SelectionType type{};

	float scale{1.f};
	float min_scale{0.1f};
	float max_scale{4.f};
};

} // namespace pi
