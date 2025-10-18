
#pragma once

#include <fornani/shader/LightShader.hpp>
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
	std::optional<std::reference_wrapper<world::Map>> get_map() override { return map; }
	void acquire_item(ServiceProvider& svc, player::Player& player, int modifier);
	void acquire_gun(ServiceProvider& svc, player::Player& player, int modifier);
	void remove_gun(ServiceProvider& svc, player::Player& player, int modifier);
	void read_item(int id);

	world::Map map;
	bool show_colliders{false};
	std::optional<std::unique_ptr<gui::VendorDialog>> vendor_dialog{};
	std::optional<std::unique_ptr<gui::PauseWindow>> pause_window{};
	std::optional<std::unique_ptr<gui::InventoryWindow>> inventory_window{};

  private:
	std::optional<LightShader> m_world_shader{};
	std::optional<LightShader> m_gui_shader{};
	ServiceProvider* m_services;

	util::Cooldown m_enter_room;
	util::Cooldown m_loading;
};

} // namespace fornani::automa
