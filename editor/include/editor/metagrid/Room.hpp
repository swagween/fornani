
#pragma once

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <fornani/graphics/Colors.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/setup/MapData.hpp>
#include <fornani/utils/Flaggable.hpp>
#include <fornani/utils/ID.hpp>

namespace fornani::automa {
struct ServiceProvider;
}
namespace pi {

enum class RoomFlags { include_in_minimap, use_template };

constexpr auto spacing_v = 32.f;
constexpr sf::Color excluded_room_color_v{120, 80, 80, 20};
constexpr sf::Color highlighted_excluded_room_color_v{120, 100, 80, 100};
constexpr sf::Color room_color_v{79, 22, 32, 220};
constexpr sf::Color highighted_room_color_v{79, 22, 32, 180};

class Room : public fornani::Flaggable<RoomFlags> {
  public:
	Room(fornani::automa::ServiceProvider& svc, fornani::data::MapData& in);

	void update(sf::Vector2f mouse_position) { m_highlighted = m_box.getGlobalBounds().contains(mouse_position); }
	bool serialize(fornani::automa::ServiceProvider& svc);

	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void set_position(sf::Vector2i const to) { m_position = to; }

	[[nodiscard]] auto get_board_position() const -> sf::Vector2f { return sf::Vector2f{m_position} * spacing_v; }
	[[nodiscard]] auto is_highlighted() const -> bool { return m_highlighted; }
	[[nodiscard]] auto get_region() const -> std::string_view { return m_data->region_label; }
	[[nodiscard]] auto get_filename() const -> std::string { return m_data->room_label + ".json"; }
	[[nodiscard]] auto get_label() const -> std::string { return m_label.getString(); }
	[[nodiscard]] auto get_biome() const -> std::string { return m_biome.getString(); }

	fornani::ID id;
	bool no_border{};

  private:
	sf::RectangleShape m_box{};
	sf::RenderTexture m_texture{};
	sf::Vector2i m_position{};
	sf::Text m_label;
	sf::Text m_biome;
	bool m_highlighted{};

	fornani::io::Logger m_logger{"Room"};

	fornani::data::MapData* m_data;
};

} // namespace pi
