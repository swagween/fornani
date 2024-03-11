
#pragma once

#include "../GameState.hpp"

namespace automa {

class MainMenu : public GameState {

  public:
	std::unordered_map<MenuSelection, int> menu_selection_id{
		{MenuSelection::new_game, 0}, {MenuSelection::load_game, 1}, {MenuSelection::options, 2}};

	MainMenu();

	void init(ServiceProvider& svc, std::string const& load_path);
	void setTilesetTexture(ServiceProvider& svc, sf::Texture& t);
	void handle_events(ServiceProvider& svc, sf::Event& event);
	void tick_update(ServiceProvider& svc);
	void frame_update(ServiceProvider& svc);
	void render(ServiceProvider& svc, sf::RenderWindow& win);

	sf::Sprite title{};
	std::vector<sf::Sprite> title_assets{};

	int selection_width{};
	int selection_buffer{};
	int title_buffer{};
	int top_buffer{};
	int middle{};

	sf::IntRect new_rect{};
	sf::IntRect load_rect{};
	sf::IntRect options_rect{};

	vfx::Gravitator left_dot{};
	vfx::Gravitator right_dot{};
	sf::Vector2<float> dot_pad{24.f, 8.f};
};

}
