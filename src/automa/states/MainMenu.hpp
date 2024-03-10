
#pragma once

#include "../GameState.hpp"

namespace automa {

class MainMenu : public GameState {

  public:
	std::unordered_map<MenuSelection, int> menu_selection_id{
		{MenuSelection::new_game, 0}, {MenuSelection::load_game, 1}, {MenuSelection::options, 2}};

	MainMenu(services::ServiceLocator& svc);

	void init(std::string const& load_path, services::ServiceLocator& svc);
	void setTilesetTexture(sf::Texture& t);
	void handle_events(sf::Event& event, services::ServiceLocator& svc);
	void tick_update(services::ServiceLocator& svc);
	void frame_update(services::ServiceLocator& svc);
	void render(sf::RenderWindow& win, services::ServiceLocator& svc);

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
