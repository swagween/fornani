#pragma once

#include <SFML/Graphics.hpp>
#include <djson\json.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/utils/BitFlags.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::world {
class Map;
}

namespace fornani::gui {

constexpr auto num_resolution_levels_v = 3;

enum class MapTextureFlags : std::uint8_t { current };
enum class Resolution : std::uint8_t { high, medium, low };

struct MapTextureLayer {
	sf::RenderTexture center_texture{};
	sf::RenderTexture border_texture{};
};

class MapTexture {
  public:
	MapTexture() = default;
	explicit MapTexture(automa::ServiceProvider& svc);
	void bake(dj::Json const& in);
	void bake(automa::ServiceProvider& svc, world::Map& map, int room, float scale, bool current = false, bool undiscovered = false);
	void set_current() { m_flags.set(MapTextureFlags::current); }
	void set_resolution(Resolution to) { m_current_resolution = to; }

	[[nodiscard]] auto is_current() const -> bool { return m_flags.test(MapTextureFlags::current); }
	[[nodiscard]] auto to_ignore() const -> bool { return m_ignore; }
	[[nodiscard]] auto get_id() const -> int { return m_room_id; }
	[[nodiscard]] auto get_scale() const -> float { return m_scale; }

	sf::RenderTexture& get(bool border = false);
	sf::Vector2f get_position();
	sf::Vector2f get_dimensions() const;

  private:
	MapTextureLayer& current_layer() { return static_cast<int>(m_current_resolution) < num_resolution_levels_v ? m_layers[static_cast<std::size_t>(m_current_resolution)] : m_layers[0]; }
	std::array<MapTextureLayer, num_resolution_levels_v> m_layers{};
	sf::RectangleShape m_tile_box{};
	sf::Color m_tile_color{};
	sf::Color m_border_color{};
	sf::Vector2<int> m_global_offset{};
	sf::Vector2f m_map_dimensions{};
	util::BitFlags<MapTextureFlags> m_flags{};
	bool m_ignore{};
	int m_room_id{};
	float m_scale{};
	Resolution m_current_resolution{};

	io::Logger m_logger{"graphics"};
};

} // namespace fornani::gui
