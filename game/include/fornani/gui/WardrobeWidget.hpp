
#pragma once
#include <string_view>

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace gui {
class WardrobeWidget {
  public:
	WardrobeWidget(const automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void set_position(sf::Vector2<float> pos) { position = pos; }
	[[nodiscard]] auto get_sprite() -> sf::Sprite& { return out_nani; }

  private:
	struct {
		sf::Sprite base;
		sf::Sprite shirt;
		sf::Sprite pants;
		sf::Sprite hairstyle;
	} sprites;
	sf::RenderTexture nani{};
	sf::Sprite out_nani;
	sf::RectangleShape background{};
	sf::Vector2<float> position{};
	sf::Vector2<float> dimensions{128.f, 256.f};

	fornani::io::Logger m_logger{"gui"};
};

} // namespace gui
