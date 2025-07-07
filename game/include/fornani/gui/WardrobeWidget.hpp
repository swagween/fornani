
#pragma once

#include "fornani/entities/player/Wardrobe.hpp"

#include <fornani/io/Logger.hpp>

#include <SFML/Graphics.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
} // namespace fornani::player

namespace fornani::gui {

class WardrobeWidget {
  public:
	explicit WardrobeWidget(automa::ServiceProvider& svc);
	void update(player::Player& player);
	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void set_position(sf::Vector2f pos) { position = pos; }
	[[nodiscard]] auto get_sprite() -> sf::Sprite& { return out_nani; }

  private:
	[[nodiscard]] auto f_dimensions() const -> sf::Vector2f { return sf::Vector2f{m_dimensions}; }
	[[nodiscard]] auto get_lookup(player::ApparelType type, player::Player& player) const -> sf::IntRect;
	sf::Sprite m_base;
	sf::Sprite m_outfit;
	sf::RenderTexture nani{};
	sf::Sprite out_nani;
	sf::RectangleShape background{};
	sf::Vector2f position{};
	sf::Vector2i m_dimensions{64, 128};

	io::Logger m_logger{"gui"};
};

} // namespace fornani::gui
