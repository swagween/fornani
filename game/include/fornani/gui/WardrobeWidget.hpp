
#pragma once

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::gui {
class WardrobeWidget {
  public:
	explicit WardrobeWidget(automa::ServiceProvider const& svc);
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

	io::Logger m_logger{"gui"};
};

} // namespace fornani::gui
