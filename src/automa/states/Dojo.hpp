
#pragma once

#include "../GameState.hpp"

namespace automa {

class Dojo : public GameState {
  public:
	Dojo(services::ServiceLocator& svc);
	void init(std::string const& load_path, services::ServiceLocator& svc);
	void setTilesetTexture(sf::Texture& t);
	void handle_events(sf::Event& event, services::ServiceLocator& svc);
	void tick_update(services::ServiceLocator& svc);
	void frame_update(services::ServiceLocator& svc);
	void render(sf::RenderWindow& win, services::ServiceLocator& svc);

	world::Map map{};
	sf::Texture tileset{};
	std::vector<sf::Sprite> tileset_sprites{};
	bool show_colliders{false};
	int x{0};
};

} // namespace automa
