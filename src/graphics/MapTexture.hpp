#pragma once

#include <SFML/Graphics.hpp>
#include <string_view>
#include "../utils/BitFlags.hpp"

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
	sf::Sprite sprite();
	sf::RenderTexture& get();
	sf::Vector2<float> get_position();
	sf::RectangleShape tile_box{};
	sf::RectangleShape plat_box{};
	sf::RectangleShape portal_box{};
	sf::RectangleShape breakable_box{};
	sf::RectangleShape save_box{};
	sf::RectangleShape curtain{};

  private:
	sf::RenderTexture map_texture{};
	sf::Color tile_color{};
	sf::Vector2<int> global_offset{};
	util::BitFlags<MapTextureFlags> flags{};
};

} // namespace gui
