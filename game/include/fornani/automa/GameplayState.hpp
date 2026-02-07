
#pragma once

#include <fornani/automa/GameState.hpp>
#include <fornani/events/Subscription.hpp>
#include <fornani/gui/VendorDialog.hpp>
#include <fornani/utils/Flaggable.hpp>

namespace fornani::automa {

enum class GameplayStateFlags { early_tick_return };

class GameplayState : public GameState, public Flaggable<GameplayStateFlags> {
  public:
	GameplayState(ServiceProvider& svc, player::Player& player, std::string_view scene, int room_number);

	virtual void tick_update([[maybe_unused]] ServiceProvider& svc, capo::IEngine& engine);
	virtual void render([[maybe_unused]] ServiceProvider& svc, [[maybe_unused]] sf::RenderWindow& win);
	std::optional<std::reference_wrapper<world::Map>> get_map() override { return m_map.value(); }

  protected:
	virtual void pause(ServiceProvider& svc);
	void play_song_by_id(int id);

  protected:
	std::optional<LightShader> p_world_shader{};
	std::optional<LightShader> p_gui_shader{};
	std::optional<std::unique_ptr<gui::PauseWindow>> p_pause_window{};
	std::optional<std::unique_ptr<gui::VendorDialog>> p_vendor_dialog{};
	std::optional<std::unique_ptr<gui::InventoryWindow>> p_inventory_window{};
	std::shared_ptr<Slot const> p_slot{std::make_shared<Slot const>()};
	ServiceProvider* p_services;
};

} // namespace fornani::automa
