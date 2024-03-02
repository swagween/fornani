
#pragma once

#include "../GameState.hpp"

namespace automa {

class Dojo : public GameState {
  public:
	Dojo();
	void init(std::string const& load_path);
	void setTilesetTexture(sf::Texture& t);
	void handle_events(sf::Event& event);
	void tick_update();
	void frame_update();
	void render(sf::RenderWindow& win);

	world::Map map{};
	sf::Texture tileset{};
	std::vector<sf::Sprite> tileset_sprites{};
	bool show_colliders{false};
	int x{0};
};

} // namespace automa
