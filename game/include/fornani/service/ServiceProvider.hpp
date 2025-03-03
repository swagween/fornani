#pragma once
#include "fornani/audio/MusicPlayer.hpp"
#include "fornani/audio/Soundboard.hpp"
#include "fornani/automa/MenuController.hpp"
#include "fornani/automa/StateController.hpp"
#include "fornani/core/AssetManager.hpp"
#include "fornani/core/SoundManager.hpp"
#include "fornani/graphics/CameraController.hpp"
#include "fornani/graphics/Style.hpp"
#include "fornani/io/Logger.hpp"
#include "fornani/setup/AccessibilityService.hpp"
#include "fornani/setup/ControllerMap.hpp"
#include "fornani/setup/DataManager.hpp"
#include "fornani/setup/Tables.hpp"
#include "fornani/setup/TextManager.hpp"
#include "fornani/setup/Version.hpp"
#include "fornani/setup/WindowManager.hpp"
#include "fornani/story/QuestTracker.hpp"
#include "fornani/story/StatTracker.hpp"
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Constants.hpp"
#include "fornani/utils/Stopwatch.hpp"
#include "fornani/utils/Ticker.hpp"
#include "fornani/utils/WorldClock.hpp"

namespace fornani::automa {
enum class DebugFlags : std::uint8_t { imgui_overlay, greyblock_mode, greyblock_trigger, demo_mode, debug_mode };
enum class AppFlags : std::uint8_t { fullscreen, tutorial, in_game };
enum class StateFlags : std::uint8_t { hide_hud, no_menu };
struct PlayerDat {
	void set_piggy_id(int const id) { piggy_id = id; }
	void unpiggy() { drop_piggy = true; }
	int piggy_id{};
	bool drop_piggy{};
};
struct MapDebug {
	int active_projectiles{};
};
// TODO: Honestly the entire ServiceProvider needs to go but it's gonna be a slow process of eliminating it.
// TODO: Convert ServiceProvider to a MonoInstance
/*
 * Here
 */
struct ServiceProvider {
	ServiceProvider(char** argv, Version& version, WindowManager& window) : finder(argv), text{finder}, data(*this, argv), version(&version), window(&window), assets{finder}, sounds{finder} {};

	util::Stopwatch stopwatch{}; // TODO: Remove. Make Free-Standing.
	data::ResourceFinder finder;
	lookup::Tables tables{};
	util::Constants constants{}; // TODO: Remove. Make Free-Standing.
	data::TextManager text;
	data::DataManager data;
	Version* version;	   // TODO: Remove. Make Free-Standing.
	WindowManager* window; // TODO: Move this into the Application class and make it into a MonoInstance
	core::SoundManager sounds;
	core::AssetManager assets;
	config::ControllerMap controller_map{*this};
	style::Style styles{};
	util::BitFlags<DebugFlags> debug_flags{};
	util::BitFlags<AppFlags> app_flags{};
	util::BitFlags<StateFlags> state_flags{};
	util::Ticker ticker{}; // TODO: Remove. Make Free-Standing. This one is gonna be hard to remove as the underlying logic needs to change for many functions.
	WorldClock world_clock{};
	StateController state_controller{};
	MenuController menu_controller{};
	audio::Soundboard soundboard{*this}; // TODO: Remove. Make Free-Standing. Maybe?
	audio::MusicPlayer music{};			 // TODO: Remove. Make Free-Standing. Maybe?
	QuestTracker quest{};
	StatTracker stats{};
	PlayerDat player_dat{};
	MapDebug map_debug{};
	util::Logger logger{};
	config::AccessibilityService a11y{};
	graphics::CameraController camera_controller{};
	int current_room{}; // TODO: Find a better way to deliver this info to the MiniMap.

	// debug stuff
	int out_value{};

	// TODO: Much of this honestly should be handled by different areas of the project instead of by the ServiceProvider.
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
} // namespace fornani::automa
