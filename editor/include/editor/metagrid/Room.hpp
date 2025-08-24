
#pragma once

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <fornani/graphics/Colors.hpp>
#include <fornani/setup/MapData.hpp>
#include <fornani/utils/ID.hpp>

namespace fornani::automa {
struct ServiceProvider;
}
namespace pi {

constexpr auto spacing_v = 24.f;
constexpr sf::Color excluded_room_color_v{120, 80, 80, 20};
constexpr sf::Color highlighted_excluded_room_color_v{120, 100, 80, 60};
constexpr sf::Color room_color_v{80, 120, 180, 60};
constexpr sf::Color highighted_room_color_v{60, 130, 190, 90};

class Room {
  public:
	Room(fornani::automa::ServiceProvider& svc, fornani::data::MapData& in);

	void update(sf::Vector2f mouse_position) { m_highlighted = m_box.getGlobalBounds().contains(mouse_position); }
	bool serialize(fornani::automa::ServiceProvider& svc);

	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void toggle_minimap() { m_include_in_minimap = !m_include_in_minimap; }
	void set_position(sf::Vector2i const to) { m_position = to; }

	[[nodiscard]] auto get_board_position() const -> sf::Vector2f { return sf::Vector2f{m_position} * spacing_v; }
	[[nodiscard]] auto is_highlighted() const -> bool { return m_highlighted; }
	[[nodiscard]] auto is_minimap() const -> bool { return m_include_in_minimap; }
	[[nodiscard]] auto get_region() const -> std::string_view { return m_data->region_label; }
	[[nodiscard]] auto get_filename() const -> std::string { return m_data->room_label + ".json"; }
	[[nodiscard]] auto get_label() const -> std::string { return m_label.getString(); }
	[[nodiscard]] auto get_biome() const -> std::string { return m_biome.getString(); }

	fornani::ID id;

  private:
	sf::RectangleShape m_box{};
	sf::Vector2i m_position{};
	sf::Text m_label;
	sf::Text m_biome;
	bool m_highlighted{};
	bool m_include_in_minimap{};

	fornani::data::MapData* m_data;
};

} // namespace pi
