
#pragma once

#include "Background.hpp"
#include "EntitySet.hpp"
#include "Map.hpp"
#include "editor/util/BitFlags.hpp"
#include "editor/util/SelectBox.hpp"
#include "fornani/graphics/CameraController.hpp"
#include "fornani/utils/Constants.hpp"
#include "fornani/world/Map.hpp"

#include <deque>
#include <filesystem>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::data {
class ResourceFinder;
}

namespace pi {

enum class StyleType : std::uint8_t { firstwind, overturned, base, factory, greatwing, END };

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
constexpr inline std::size_t max_undo_states_v{64};

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

	[[nodiscard]] auto get_label_char() -> char const*& { return label_c_str; };
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
	Canvas(fornani::data::ResourceFinder& finder, SelectionType type, StyleType style = StyleType::firstwind, Backdrop backdrop = Backdrop::black, int num_layers = default_num_layers_v);
	Canvas(fornani::data::ResourceFinder& finder, sf::Vector2<std::uint32_t> dim, SelectionType type, StyleType style, Backdrop backdrop, int num_layers = default_num_layers_v);
	void update(Tool& tool);
	void render(sf::RenderWindow& win, sf::Sprite& tileset);
	bool load(fornani::automa::ServiceProvider& svc, fornani::data::ResourceFinder& finder, std::string const& region, std::string const& room_name, bool local = false);
	bool save(fornani::data::ResourceFinder& finder, std::string const& region, std::string const& room_name);
	void clear();
	void save_state(Tool& tool, bool force = false);
	void undo();
	void redo();
	void clear_redo_states();
	void hover();
	void unhover();
	void move(sf::Vector2f distance);
	void set_position(sf::Vector2f to_position);
	void set_origin(sf::Vector2f to_origin);
	void set_offset_from_center(sf::Vector2f offset);
	void set_scale(float to_scale);
	void resize(sf::Vector2i adjustment);
	void center(sf::Vector2f point);
	void constrain(sf::Vector2f bounds);
	void zoom(float amount);
	void set_backdrop_color(sf::Color color);
	void set_grid_texture();
	void activate_middleground();
	Map& get_layers();
	sf::Vector2<int> get_tile_coord(int lookup);
	void set_property(fornani::world::MapProperties to_set) { m_map_properties.set(to_set); }
	void reset_property(fornani::world::MapProperties to_reset) { m_map_properties.reset(to_reset); }

	[[nodiscard]] auto get_selection_type() const -> SelectionType { return type; }
	[[nodiscard]] auto states_empty() const -> bool { return map_states.empty(); }
	[[nodiscard]] auto is_palette() const -> bool { return type == SelectionType::palette; }
	[[nodiscard]] auto hovered() const -> bool { return state.test(CanvasState::hovered); }
	[[nodiscard]] auto editable() const -> bool { return properties.test(CanvasProperties::editable); }
	[[nodiscard]] auto chunk_dimensions() const -> sf::Vector2<std::uint32_t> { return dimensions / u_native_chunk_size(); }
	[[nodiscard]] auto get_position() const -> sf::Vector2f { return position; }
	[[nodiscard]] auto get_scaled_position() const -> sf::Vector2f { return position / scale; }
	[[nodiscard]] auto get_real_dimensions() const -> sf::Vector2f { return real_dimensions * scale; }
	[[nodiscard]] auto get_center() const -> sf::Vector2f { return real_dimensions * 0.5f; }
	[[nodiscard]] auto get_origin() const -> sf::Vector2f { return origin; }
	[[nodiscard]] auto get_offset_from_center() const -> sf::Vector2f { return offset_from_center; }
	[[nodiscard]] auto get_scaled_center() const -> sf::Vector2f { return real_dimensions * 0.5f * scale; }
	[[nodiscard]] auto u_native_chunk_size() const -> std::uint32_t { return static_cast<std::uint32_t>(chunk_size_v); }
	[[nodiscard]] auto i_native_chunk_size() const -> int { return chunk_size_v; }
	[[nodiscard]] auto f_native_chunk_size() const -> float { return static_cast<float>(chunk_size_v); }
	[[nodiscard]] auto u_cell_size() const -> std::uint32_t { return static_cast<std::uint32_t>(i_cell_size()); }
	[[nodiscard]] auto i_cell_size() const -> int { return static_cast<int>(f_cell_size()); }
	[[nodiscard]] auto f_cell_size() const -> float { return f_native_cell_size() * scale; }
	[[nodiscard]] auto f_chunk_size() const -> float { return f_native_chunk_size() * scale; }
	[[nodiscard]] auto f_native_cell_size() const -> float { return 32.f; }
	[[nodiscard]] auto get_scale() const -> float { return scale; }
	[[nodiscard]] auto get_scale_vec() const -> sf::Vector2f { return fornani::constants::f_scale_vec * scale; }
	[[nodiscard]] auto get_i_style() const -> int { return static_cast<int>(tile_style.get_type()); }
	[[nodiscard]] auto within_zoom_limits(float delta) const -> bool { return get_scale() + delta >= min_scale && get_scale() + delta <= max_scale; }
	[[nodiscard]] auto within_bounds(sf::Vector2f const& point) const -> bool { return point.x > position.x && point.x < real_dimensions.x + position.x && point.y > position.y && point.y < real_dimensions.y + position.y; }
	[[nodiscard]] auto undo_states_size() const -> std::size_t { return map_states.size(); }
	[[nodiscard]] auto redo_states_size() const -> std::size_t { return redo_states.size(); }
	[[nodiscard]] auto middleground() const -> int { return map_states.back().get_middleground(); }
	[[nodiscard]] auto last_layer() const -> int { return static_cast<int>(map_states.back().layers.size() - 1); }

	void replace_tile(std::uint32_t from, std::uint32_t to, int layer_index);
	void edit_tile_at(int i, int j, int new_val, int layer_index);
	void erase_at(int i, int j, int layer_index);
	int tile_val_at(int i, int j, int layer);
	int tile_val_at_scaled(int i, int j, int layer);
	sf::Vector2f get_tile_position_at(int i, int j, int layer = 0);
	Tile& get_tile_at(int i, int j, int layer = 0);

	// layers
	sf::Vector2<std::uint32_t> dimensions{};
	sf::Vector2<int> metagrid_coordinates{};
	sf::Vector2f m_player_start{};

	struct {
		bool show_grid{true};
		bool show_all_layers{true};
		bool show_current_layer{false};
		bool show_obscured_layer{false};
		bool show_reverse_obscured_layer{false};
		bool show_indicated_layers{true};
		bool show_entities{true};
		bool show_background{};
	} flags{};

	EntitySet entities;
	dj::Json metadata{};
	Style tile_style;

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

	std::uint32_t room_id{};

	bool minimap{};

  private:
	sf::Vector2f position{};
	sf::RenderTexture grid_texture{};
	sf::Vector2f origin{};
	sf::Vector2f real_dimensions{};
	sf::Vector2f offset_from_center{};
	std::deque<Map> map_states{};
	std::deque<Map> redo_states{};
	util::BitFlags<CanvasState> state{};
	util::BitFlags<CanvasProperties> properties{};
	util::BitFlags<fornani::world::MapProperties> m_map_properties{};
	sf::RectangleShape box{};
	sf::RectangleShape gridbox{};
	sf::RectangleShape chunkbox{};
	sf::RectangleShape border{};

	SelectionType type{};

	float scale{1.f};
	float min_scale{0.1f};
	float max_scale{4.f};

	fornani::io::Logger m_logger{"pioneer"};
};

} // namespace pi
