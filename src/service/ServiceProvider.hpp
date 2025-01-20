#pragma once
#include "../audio/MusicPlayer.hpp"
#include "../audio/Soundboard.hpp"
#include "../automa/MenuController.hpp"
#include "../automa/StateController.hpp"
#include "../graphics/Style.hpp"
#include "../setup/AccessibilityService.hpp"
#include "../setup/AssetManager.hpp"
#include "../setup/ControllerMap.hpp"
#include "../setup/DataManager.hpp"
#include "../setup/Tables.hpp"
#include "../setup/TextManager.hpp"
#include "../setup/WindowManager.hpp"
#include "../story/QuestTracker.hpp"
#include "../story/StatTracker.hpp"
#include "../utils/BitFlags.hpp"
#include "../utils/Constants.hpp"
#include "../utils/Random.hpp"
#include "../utils/Stopwatch.hpp"
#include "../utils/Ticker.hpp"
#include "../utils/WorldClock.hpp"
#include "../utils/Logger.hpp"
#include "../setup/Version.hpp"

namespace automa {
enum class DebugFlags { imgui_overlay, greyblock_mode, greyblock_trigger, demo_mode, debug_mode };
enum class AppFlags { fullscreen, tutorial, in_game };
enum class StateFlags { hide_hud, no_menu };
struct PlayerDat {
	void set_piggy_id(int id) { piggy_id = id; }
	void unpiggy() { drop_piggy = true; }
	int piggy_id{};
	bool drop_piggy{};
};
struct MapDebug {
	int active_projectiles{};
};
struct ServiceProvider {
	ServiceProvider(char** argv, fornani::Version& version, fornani::WindowManager& window) : finder(argv), data(*this, argv), version(version), assets{finder}, text{finder}, window(&window) {};

	data::ResourceFinder finder;
	lookup::Tables tables{};
	data::TextManager text;
	data::DataManager data;
	fornani::Version version{};
	fornani::WindowManager* window;
	asset::AssetManager assets;
	config::ControllerMap controller_map{*this};
	style::Style styles{};
	util::BitFlags<DebugFlags> debug_flags{};
	util::BitFlags<AppFlags> app_flags{};
	util::BitFlags<StateFlags> state_flags{};
	util::Random random{};
	util::Ticker ticker{};
	fornani::WorldClock world_clock{};
	util::Constants constants{};
	StateController state_controller{};
	MenuController menu_controller{};
	audio::Soundboard soundboard{*this};
	audio::MusicPlayer music{};
	fornani::QuestTracker quest{};
	fornani::StatTracker stats{};
	PlayerDat player_dat{};
	MapDebug map_debug{};
	util::Logger logger{};
	config::AccessibilityService a11y{};

	// debug stuff
	util::Stopwatch stopwatch{};

	void toggle_fullscreen() { fullscreen() ? app_flags.reset(AppFlags::fullscreen) : app_flags.set(AppFlags::fullscreen); }
	void toggle_tutorial() { tutorial() ? app_flags.reset(AppFlags::tutorial) : app_flags.set(AppFlags::tutorial); }
	void toggle_debug() { debug_mode() ? debug_flags.reset(DebugFlags::debug_mode) : debug_flags.set(DebugFlags::debug_mode); }
	void set_fullscreen(bool flag) { flag ? app_flags.set(AppFlags::fullscreen) : app_flags.reset(AppFlags::fullscreen); }
	void set_tutorial(bool flag) { flag ? app_flags.set(AppFlags::tutorial) : app_flags.reset(AppFlags::tutorial); }

	[[nodiscard]] auto fullscreen() const -> bool { return app_flags.test(AppFlags::fullscreen); }
	[[nodiscard]] auto tutorial() const -> bool { return app_flags.test(AppFlags::tutorial); }
	[[nodiscard]] auto in_game() const -> bool { return app_flags.test(AppFlags::in_game); }
	[[nodiscard]] auto hide_hud() const -> bool { return state_flags.test(StateFlags::hide_hud); }
	[[nodiscard]] auto no_menu() const -> bool { return state_flags.test(StateFlags::no_menu); }
	[[nodiscard]] auto demo_mode() const -> bool { return debug_flags.test(DebugFlags::demo_mode); }
	[[nodiscard]] auto greyblock_mode() const -> bool { return debug_flags.test(DebugFlags::greyblock_mode); }
	[[nodiscard]] auto debug_mode() const -> bool { return debug_flags.test(DebugFlags::debug_mode); }
	[[nodiscard]] auto death_mode() const -> bool { return state_controller.actions.test(Actions::death_mode); }
	[[nodiscard]] auto in_window(sf::Vector2<float> point, sf::Vector2<float> dimensions) const -> bool { return window->in_window(point, dimensions); }
};
} // namespace automa