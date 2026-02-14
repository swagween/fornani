
#pragma once

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <editor/canvas/Map.hpp>
#include <editor/util/BitFlags.hpp>
#include <editor/util/SelectBox.hpp>
#include <fornani/entity/EntitySet.hpp>
#include <fornani/graphics/Background.hpp>
#include <fornani/graphics/Biome.hpp>
#include <fornani/graphics/CameraController.hpp>
#include <fornani/utils/Constants.hpp>
#include <fornani/world/Map.hpp>
#include <deque>
#include <filesystem>
#include <string>
#include <vector>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani {
class ResourceFinder;
}

namespace pi {

enum class CanvasProperties { editable };
enum class CanvasState { hovered };

constexpr inline int chunk_size_v{16};
constexpr inline int default_num_layers_v{8};
constexpr inline int default_middleground_v{4};
constexpr inline std::size_t max_undo_states_v{64};

class Tool;

class Canvas {

  public:
	Canvas(fornani::automa::ServiceProvider& svc, SelectionType type, fornani::Biome biome, std::string_view backdrop = "black", int num_layers = default_num_layers_v);
	Canvas(fornani::automa::ServiceProvider& svc, sf::Vector2<std::uint32_t> dim, SelectionType type, fornani::Biome biome, std::string_view backdrop, int num_layers = default_num_layers_v);
	void update(Tool& tool);
	void render(sf::RenderWindow& win, sf::Sprite& tileset);
	bool load(fornani::automa::ServiceProvider& svc, fornani::ResourceFinder& finder, std::string const& region, std::string const& room_name, bool local = false);
	bool save(fornani::ResourceFinder& finder, std::string const& region, std::string const& room_name);
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
	void set_music(std::string_view to) { m_attributes.music = to; }
	void set_ambience(std::string_view to) { m_attributes.ambience = to; }
	void add_atmosphere(std::string_view to) { m_attributes.atmosphere.add(to.data()); }
	void remove_atmosphere(std::string_view to) { m_attributes.atmosphere.remove(to.data()); }

	Map& get_layers();
	Layer& get_active_layer();
	sf::Vector2<int> get_tile_coord(int lookup);

	void set_property(fornani::world::MapProperties to_set) { m_attributes.properties.set(to_set); }
	void reset_property(fornani::world::MapProperties to_reset) { m_attributes.properties.reset(to_reset); }

	[[nodiscard]] auto test_property(fornani::world::MapProperties to_test) const -> bool { return m_attributes.properties.test(to_test); }

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
	[[nodiscard]] auto get_i_style() const -> int { return biome.get_id(); }
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

	float darken_factor{};

	fornani::EntitySet entities;
	dj::Json metadata{};

	std::unique_ptr<fornani::graphics::Background> background{};
	fornani::Biome biome{};

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

	bool m_use_template{};

  private:
	fornani::world::MapAttributes m_attributes{};

	sf::Vector2f position{};
	sf::RenderTexture grid_texture{};
	sf::Vector2f origin{};
	sf::Vector2f real_dimensions{};
	sf::Vector2f offset_from_center{};
	std::deque<Map> map_states{};
	std::deque<Map> redo_states{};
	util::BitFlags<CanvasState> state{};
	util::BitFlags<CanvasProperties> properties{};
	sf::RectangleShape box{};
	sf::RectangleShape gridbox{};
	sf::RectangleShape chunkbox{};
	sf::RectangleShape border{};

	SelectionType type{};

	fornani::automa::ServiceProvider* m_services;

	float scale{1.f};
	float min_scale{0.1f};
	float max_scale{4.f};

	fornani::io::Logger m_logger{"pioneer"};
};

} // namespace pi
