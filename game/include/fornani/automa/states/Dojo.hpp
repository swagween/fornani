
#pragma once

#include "fornani/automa/GameState.hpp"
#include "fornani/gui/VendorDialog.hpp"
#include "fornani/level/Camera.hpp"

#include <memory>
#include <optional>

namespace fornani::automa {

class Dojo final : public GameState {
  public:
	Dojo(ServiceProvider& svc, player::Player& player, std::string_view scene = "", int room_number = 0, std::string_view room_name = "");
	void tick_update(ServiceProvider& svc) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, sf::RenderWindow& win) override;
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
	std::optional<gui::PauseWindow> pause_window{};
	std::optional<std::unique_ptr<gui::InventoryWindow>> inventory_window{};

	// shape::Collider A{};
	// shape::Collider B{};
	// shape::CircleCollider circle{16.f};
};

} // namespace fornani::automa
