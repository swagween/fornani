#pragma once
#include "../utils/BitFlags.hpp"
#include "../setup/WindowManager.hpp"
#include "../setup/AssetManager.hpp"
#include "../setup/DataManager.hpp"
#include "../setup/TextManager.hpp"
#include "../setup/Tables.hpp"
#include "../automa/StateController.hpp"
#include "../automa/MenuController.hpp"
#include "../graphics/Style.hpp"
#include "../utils/Random.hpp"
#include "../utils/Ticker.hpp"
#include "../utils/Constants.hpp"
#include "../audio/Soundboard.hpp"
#include "../audio/MusicPlayer.hpp"
#include "../setup/ControllerMap.hpp"
#include "../utils/Stopwatch.hpp"
#include "../story/QuestTracker.hpp"
#include "../story/StatTracker.hpp"

namespace automa {
enum class DebugFlags { imgui_overlay, greyblock_mode, greyblock_trigger, demo_mode };
enum class AppFlags { fullscreen, tutorial, in_game };
enum class StateFlags { hide_hud, no_menu };
struct ServiceProvider {
	fornani::WindowManager* window;
	asset::AssetManager assets{};
	data::DataManager data{*this};
	data::TextManager text{};
	config::ControllerMap controller_map{*this};
	style::Style styles{};
	util::BitFlags<DebugFlags> debug_flags{};
	util::BitFlags<AppFlags> app_flags{};
	util::BitFlags<StateFlags> state_flags{};
	util::Random random{};
	util::Ticker ticker{};
	util::Constants constants{};
	lookup::Tables tables{};
	StateController state_controller{};
	MenuController menu_controller{};
	audio::Soundboard soundboard{};
	audio::MusicPlayer music{};
	fornani::QuestTracker quest{};
	fornani::StatTracker stats{};

	// debug stuff
	util::Stopwatch stopwatch{};
	sf::Text debug_text{};

	void toggle_fullscreen() { fullscreen() ? app_flags.reset(AppFlags::fullscreen) : app_flags.set(AppFlags::fullscreen); }
	void toggle_tutorial() { tutorial() ? app_flags.reset(AppFlags::tutorial) : app_flags.set(AppFlags::tutorial); }
	void set_fullscreen(bool flag) { flag ? app_flags.set(AppFlags::fullscreen) : app_flags.reset(AppFlags::fullscreen); }
	void set_tutorial(bool flag) { flag ? app_flags.set(AppFlags::tutorial) : app_flags.reset(AppFlags::tutorial); }

	[[nodiscard]] auto fullscreen() const -> bool { return app_flags.test(AppFlags::fullscreen); }
	[[nodiscard]] auto tutorial() const -> bool { return app_flags.test(AppFlags::tutorial); }
	[[nodiscard]] auto in_game() const -> bool { return app_flags.test(AppFlags::in_game); }
	[[nodiscard]] auto hide_hud() const -> bool { return state_flags.test(StateFlags::hide_hud); }
	[[nodiscard]] auto no_menu() const -> bool { return state_flags.test(StateFlags::no_menu); }
	[[nodiscard]] auto demo_mode() const -> bool { return debug_flags.test(DebugFlags::demo_mode); }
	[[nodiscard]] auto greyblock_mode() const -> bool { return debug_flags.test(DebugFlags::greyblock_mode); }
	[[nodiscard]] auto death_mode() const -> bool { return state_controller.actions.test(Actions::death_mode); }
};
} // namespace automa