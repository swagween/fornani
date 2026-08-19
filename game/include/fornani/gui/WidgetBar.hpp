
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/gui/Widget.hpp>
#include <fornani/utils/Flaggable.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani {
class Health;
}

namespace fornani::gui {

enum class WidgetBarFlags : std::uint8_t { needs_resize };

class WidgetBar : public Flaggable<WidgetBarFlags> {
  public:
	WidgetBar(automa::ServiceProvider& svc, int amount, sf::Vector2i dimensions, std::string_view tag, sf::Vector2f origin, float pad = 2.f, bool compress = false);
	void update(automa::ServiceProvider& svc, Health& health, bool shake = false);
	void render(sf::RenderWindow& win);
	void set_origin(sf::Vector2f const to);
	void set_position(sf::Vector2f const to) { m_position = to; }

	[[nodiscard]] auto get_position() const -> sf::Vector2f { return m_position; }
	[[nodiscard]] auto get_endpoint() const -> sf::Vector2f { return m_position + sf::Vector2f{m_widgets.size() * constants::f_scale_factor * m_dimensions.x + (m_widgets.size() * m_pad) - m_dimensions.x, -m_dimensions.y}; }

  private:
	float m_pad;
	bool m_compress{};
	std::vector<Widget> m_widgets{};
	sf::Text m_text;
	sf::Vector2f m_position{};
	sf::Vector2f m_dimensions{};
};

} // namespace fornani::gui
