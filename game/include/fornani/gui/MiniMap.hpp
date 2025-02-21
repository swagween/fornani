
#pragma once
#include <SFML/Graphics.hpp>
#include "Console.hpp"
#include "Selector.hpp"
#include "fornani/graphics/MapTexture.hpp"
#include "fornani/utils/Circuit.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::player {
class Player;
}

namespace fornani::entity {
class Portal;
}

namespace fornani::gui {

enum class ChunkType : uint8_t { top_left, top, top_right, bottom_left, bottom, bottom_right, left, right, inner };

struct Chunk {
	sf::Vector2<uint32_t> position{};
	uint64_t one_d_index{};
	sf::RectangleShape drawbox{};
	ChunkType type{};
	void generate();
};

class MiniMap {
  public:
	explicit MiniMap(automa::ServiceProvider& svc);
	void bake(automa::ServiceProvider& svc, world::Map& map, int room, bool current = false, bool undiscovered = false);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void clear_atlas();
	void toggle_scale();
	void move(sf::Vector2<float> direction);
	void center();
	[[nodiscard]] auto get_position() const -> sf::Vector2<float> { return position; }
	[[nodiscard]] auto get_extent() const -> sf::FloatRect { return extent; }
	[[nodiscard]] auto get_center_position() const -> sf::Vector2<float> { return center_position; }
	[[nodiscard]] auto get_scale() const -> float { return scale; }
	[[nodiscard]] auto get_ratio() const -> float { return ratio; }

  private:
	float scale{8.f};
	float global_ratio{};
	float ratio{};
	float speed{1.5f};
	float window_scale{};
	sf::Vector2<float> position{};
	sf::Vector2<float> previous_position{};
	sf::Vector2<float> center_position{};
	sf::Vector2<float> player_position{};
	sf::FloatRect extent{};
	sf::View view{};
	MapTexture texture;
	sf::RenderTexture minimap_texture{};
	sf::RectangleShape player_box{};
	sf::Sprite map_sprite;
	sf::RectangleShape background{};
	sf::RectangleShape border{};
	sf::RectangleShape room_border{};
	struct {
		sf::RectangleShape vert{};
		sf::RectangleShape horiz{};
	} cursor{};
	sf::Color background_color{};
	std::vector<Chunk> grid{};
	std::vector<std::unique_ptr<MapTexture>> atlas{};
	util::Circuit scalar{3};
};

} // namespace fornani::gui
