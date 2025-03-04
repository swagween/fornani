#pragma once

#include <SFML/Graphics.hpp>
#include "fornani/io/Logger.hpp"
#include "fornani/utils/BitFlags.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::world {
class Map;
}

namespace fornani::gui {

enum class MapTextureFlags : std::uint8_t { current };

class MapTexture {
  public:
	MapTexture() = default;
	explicit MapTexture(automa::ServiceProvider& svc);
	void bake(automa::ServiceProvider& svc, world::Map& map, int room, float scale, bool current = false, bool undiscovered = false);
	void set_current() { m_flags.set(MapTextureFlags::current); }
	[[nodiscard]] auto is_current() const -> bool { return m_flags.test(MapTextureFlags::current); }
	[[nodiscard]] auto to_ignore() const -> bool { return m_ignore; }
	[[nodiscard]] auto get_id() const -> int { return m_room_id; }
	[[nodiscard]] auto get_scale() const -> float { return m_scale; }
	sf::RenderTexture& get(bool border = false);
	sf::Vector2<float> get_position();
	sf::Vector2<float> get_dimensions() const;

  private:
	sf::RenderTexture m_center_texture{};
	sf::RenderTexture m_border_texture{};
	sf::RectangleShape m_tile_box{};
	sf::Color m_tile_color{};
	sf::Color m_border_color{};
	sf::Vector2<int> m_global_offset{};
	sf::Vector2<float> m_map_dimensions{};
	util::BitFlags<MapTextureFlags> m_flags{};
	bool m_ignore{};
	int m_room_id{};
	float m_scale{};

	io::Logger m_logger{"graphics"};
};

} // namespace fornani::gui
