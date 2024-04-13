
#pragma once

#include "../GameState.hpp"

namespace automa {

class MainMenu : public GameState {

  public:

	MainMenu() = default;
	MainMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int id = 0);

	void init(ServiceProvider& svc, std::string_view room = "");
	void setTilesetTexture(ServiceProvider& svc, sf::Texture& t);
	void handle_events(ServiceProvider& svc, sf::Event& event);
	void tick_update(ServiceProvider& svc);
	void frame_update(ServiceProvider& svc);
	void render(ServiceProvider& svc, sf::RenderWindow& win);

	sf::Sprite title{};
	std::vector<sf::Sprite> title_assets{};
};

}
