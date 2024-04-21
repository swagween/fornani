
#pragma once

#include "../GameState.hpp"
#include "../../level/Camera.hpp"

namespace automa {

class Dojo : public GameState {
  public:
	Dojo() = default;
	Dojo(ServiceProvider& svc, player::Player& player, std::string_view scene, int id = 0);
	void init(ServiceProvider& svc, std::string_view room = "");
	void handle_events(ServiceProvider& svc, sf::Event& event);
	void tick_update(ServiceProvider& svc);
	void frame_update(ServiceProvider& svc);
	void render(ServiceProvider& svc, sf::RenderWindow& win);
	void toggle_inventory();

	world::Map map;
	fornani::Camera camera{};
	bool show_colliders{false};
	int x{0};
};

} // namespace automa
