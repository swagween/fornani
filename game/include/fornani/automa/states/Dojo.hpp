
#pragma once

#include "fornani/automa/GameState.hpp"
#include "fornani/level/Camera.hpp"
#include "fornani/utils/Collider.hpp"
#include "fornani/gui/VendorDialog.hpp"
#include <optional>

namespace automa {

class Dojo : public GameState {
  public:
	Dojo(ServiceProvider& svc, player::Player& player, std::string_view scene = "", int room_number = 0, std::string_view room_name = "");
	void tick_update(ServiceProvider& svc);
	void frame_update(ServiceProvider& svc);
	void render(ServiceProvider& svc, sf::RenderWindow& win);
	void toggle_inventory(ServiceProvider& svc);
	void toggle_pause_menu(ServiceProvider& svc);
	void bake_maps(ServiceProvider& svc, std::vector<int> ids, bool current = false);

	world::Map map;
	world::Map gui_map;
	fornani::Camera camera{};
	bool show_colliders{false};
	bool open_vendor{};
	int x{0};
	util::Cooldown enter_room{};
	util::Cooldown loading{32};
	std::optional<gui::VendorDialog> vendor_dialog{};

	//shape::Collider A{};
	//shape::Collider B{};
	//shape::CircleCollider circle{16.f};
};

} // namespace automa
