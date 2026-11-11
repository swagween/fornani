
#pragma once

#include <fornani/automa/GameState.hpp>
#include <fornani/events/Subscription.hpp>
#include <fornani/graphics/Renderer.hpp>
#include <fornani/graphics/rewards/IRewardSequence.hpp>
#include <fornani/gui/InventoryWindow.hpp>
#include <fornani/gui/PauseWindow.hpp>
#include <fornani/gui/dialogs/IDialog.hpp>
#include <fornani/shader/FilterShader.hpp>
#include <fornani/shader/HazeShader.hpp>
#include <fornani/utils/Flaggable.hpp>

namespace fornani::automa {

enum class GameplayStateFlags : std::uint8_t { early_tick_return, transitioned_in };
enum class GameplayFlags : std::uint8_t { game_over, transitioning, open_vendor, give_item, death_console_launched, console_running, remove_item, health_increase_exit, open_builder, open_air_travel };

class GameplayState : public GameState, public Flaggable<GameplayStateFlags> {
  public:
	GameplayState(ServiceProvider& svc, player::Player& player, int room_number);

	virtual void tick_update([[maybe_unused]] ServiceProvider& svc, capo::IEngine& engine);
	virtual void render([[maybe_unused]] ServiceProvider& svc, [[maybe_unused]] sf::RenderWindow& win);
	void reload(ServiceProvider& svc, int target_state) override;
	std::optional<std::reference_wrapper<world::Map>> get_map() override { return m_map.value(); }
	virtual void handle_player_death(ServiceProvider& svc, player::Player& player) {}

  protected:
	virtual void pause(ServiceProvider& svc);
	virtual void unpause(ServiceProvider& svc);
	void play_song_by_id(int id);

  protected:
	util::BitFlags<GameplayFlags> p_flags{};
	Renderer p_renderer{};
	std::optional<Palette> m_palette{};
	std::optional<LightShader> p_entity_shader{};
	std::optional<LightShader> p_world_shader{};
	std::optional<LightShader> p_gui_shader{};
	std::optional<HazeShader> p_haze_shader{};
	std::optional<FilterShader> p_filter_shader{};
	std::optional<FilterShader> p_vignette_shader{};
	std::optional<std::unique_ptr<gui::PauseWindow>> p_pause_window{};
	std::optional<std::unique_ptr<gui::IDialog>> p_dialog{};
	std::optional<std::unique_ptr<gui::InventoryWindow>> p_inventory_window{};
	std::optional<std::unique_ptr<graphics::IRewardSequence>> p_reward_sequence{};
	std::shared_ptr<Slot const> p_slot{std::make_shared<Slot const>()};
	ServiceProvider* p_services;
};

} // namespace fornani::automa
