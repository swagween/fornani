
#pragma once

#include "../GameState.hpp"

namespace automa {

class Dojo : public GameState {
  public:
	Dojo() = default;
	Dojo(ServiceProvider& svc, player::Player& player, int id = 0);
	void init(ServiceProvider& svc, std::string_view room = "");
	void setTilesetTexture(ServiceProvider& svc, sf::Texture& t);
	void handle_events(ServiceProvider& svc, sf::Event& event);
	void tick_update(ServiceProvider& svc);
	void frame_update(ServiceProvider& svc);
	void render(ServiceProvider& svc, sf::RenderWindow& win);
	void toggle_inventory();

	world::Map map;
	sf::Texture tileset{};
	std::vector<sf::Sprite> tileset_sprites{};
	bool show_colliders{false};
	int x{0};
};

} // namespace automa
