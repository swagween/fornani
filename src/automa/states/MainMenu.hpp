
#pragma once

#include "../GameState.hpp"

namespace automa {

class MainMenu : public GameState {

  public:

	MainMenu() = default;
	MainMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int id = 0);
	~MainMenu() {}

	void init(ServiceProvider& svc, int room_number);
	void handle_events(ServiceProvider& svc, sf::Event& event);
	void tick_update(ServiceProvider& svc);
	void frame_update(ServiceProvider& svc);
	void render(ServiceProvider& svc, sf::RenderWindow& win);

	sf::Sprite title{};
	sf::Text subtitle{};
	sf::Text instruction{};
	std::vector<sf::Sprite> title_assets{};
};

}
