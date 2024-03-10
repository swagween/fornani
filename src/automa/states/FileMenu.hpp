
#pragma once

#include "../GameState.hpp"

namespace automa {

class FileMenu : public automa::GameState {

	static int const num_files{3};

  public:
	FileMenu(services::ServiceLocator& svc);
	~FileMenu();
	void init(std::string const& load_path, services::ServiceLocator& svc);
	void setTilesetTexture(sf::Texture& t);
	void handle_events(sf::Event& event, services::ServiceLocator& svc);
	void tick_update(services::ServiceLocator& svc);
	void frame_update(services::ServiceLocator& svc);
	void render(sf::RenderWindow& win, services::ServiceLocator& svc);
	void constrain_selection();

	// menu textures
	sf::RectangleShape title{};
	std::array<sf::Sprite, num_files * 2> file_text{};

	int selection_width{};
	int selection_buffer{};
	int top_buffer{};
	int middle{};
	int text_left{};
	int text_right{};
	sf::Vector2i text_dim{72, 16};
	sf::Vector2<float> zero{};

	std::array<sf::IntRect, num_files> file_rects{};
};

}
