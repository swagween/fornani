
#pragma once

#include "fornani/automa/GameState.hpp"
#include "fornani/gui/VendorDialog.hpp"
#include "fornani/world/Camera.hpp"

#include <memory>
#include <optional>

namespace fornani::automa {

class Dojo final : public GameState {
  public:
	Dojo(ServiceProvider& svc, player::Player& player, std::string_view scene = "", int room_number = 0, std::string_view room_name = "");
	void tick_update(ServiceProvider& svc, capo::IEngine& engine) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, sf::RenderWindow& win) override;
	void bake_maps(ServiceProvider& svc, std::vector<int> ids, bool current = false);
	void acquire_item(ServiceProvider& svc, player::Player& player, int modifier);
	void acquire_gun(ServiceProvider& svc, player::Player& player, int modifier);

	world::Map map;
	world::Map gui_map;
	fornani::Camera camera{};
	bool show_colliders{false};
	bool open_vendor{};
	util::Cooldown enter_room{};
	util::Cooldown loading{32};
	std::optional<std::unique_ptr<gui::VendorDialog>> vendor_dialog{};
	std::optional<std::unique_ptr<gui::PauseWindow>> pause_window{};
	std::optional<std::unique_ptr<gui::InventoryWindow>> inventory_window{};
};

} // namespace fornani::automa
