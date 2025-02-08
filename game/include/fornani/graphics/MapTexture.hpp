#pragma once

#include <SFML/Graphics.hpp>
#include "fornani/utils/BitFlags.hpp"
#include "fornani/io/Logger.hpp"

namespace automa {
struct ServiceProvider;
}

namespace world {
class Map;
}

namespace gui {

enum class MapTextureFlags { current };

class MapTexture {
  public:
	MapTexture() = default;
	MapTexture(automa::ServiceProvider& svc);
	void bake(automa::ServiceProvider& svc, world::Map& map, int room, float scale, bool current = false, bool undiscovered = false);
	void set_current() { flags.set(MapTextureFlags::current); }
	[[nodiscard]] auto is_current() const -> bool { return flags.test(MapTextureFlags::current); }
	[[nodiscard]] auto to_ignore() const -> bool { return ignore; }
	sf::Sprite sprite();
	sf::RenderTexture& get();
	sf::Vector2<float> get_position();
	sf::Vector2<float> get_dimensions() const;
	sf::RectangleShape tile_box{};
	sf::RectangleShape plat_box{};
	sf::RectangleShape portal_box{};
	sf::RectangleShape breakable_box{};
	sf::RectangleShape save_box{};
	sf::RectangleShape curtain{};

  private:
	sf::RenderTexture map_texture{};
	sf::Color tile_color{};
	sf::Color border_color{};
	sf::Vector2<int> global_offset{};
	sf::Vector2<float> map_dimensions{};
	util::BitFlags<MapTextureFlags> flags{};
	bool ignore{};

	fornani::io::Logger m_logger {"graphics"};
};

} // namespace gui
