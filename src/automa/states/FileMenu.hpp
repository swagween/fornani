
#pragma once

#include "../GameState.hpp"

namespace automa {

class FileMenu : public automa::GameState {

	static int const num_files{3};

  public:
	FileMenu();
	~FileMenu();
	void init(std::string const& load_path);
	void setTilesetTexture(sf::Texture& t);
	void handle_events(sf::Event& event);
	void logic();
	void render(sf::RenderWindow& win);
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

	std::array<sf::IntRect, num_files> file_rects{};
};

}
