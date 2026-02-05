
#pragma once

#include <fornani/audio/Ambience.hpp>
#include <fornani/audio/MusicPlayer.hpp>
#include <fornani/audio/Soundboard.hpp>
#include <fornani/automa/MenuController.hpp>
#include <fornani/automa/StateController.hpp>
#include <fornani/core/AssetManager.hpp>
#include <fornani/core/SoundManager.hpp>
#include <fornani/graphics/CameraController.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/setup/AccessibilityService.hpp>
#include <fornani/setup/DataManager.hpp>
#include <fornani/setup/TextManager.hpp>
#include <fornani/setup/Version.hpp>
#include <fornani/setup/WindowManager.hpp>
#include <fornani/story/Quest.hpp>
#include <fornani/story/QuestTracker.hpp>
#include <fornani/story/StatTracker.hpp>
#include <fornani/systems/EventRegistry.hpp>
#include <fornani/systems/InputSystem.hpp>
#include <fornani/systems/NotificationManager.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <fornani/utils/Constants.hpp>
#include <fornani/utils/Stopwatch.hpp>
#include <fornani/utils/Ticker.hpp>
#include <fornani/utils/WorldClock.hpp>
#include <fornani/utils/WorldTimer.hpp>
#include <ranges>

namespace fornani::automa {
enum class DebugFlags { imgui_overlay, greyblock_mode, greyblock_trigger, demo_mode, debug_mode };
enum class AppFlags { fullscreen, tutorial, in_game, editor, custom_map_start };
enum class StateFlags { hide_hud, no_menu, cutscene };

struct EditorSettings {
	int save_file{};
};

struct ServiceProvider {
	ServiceProvider(char** argv, Version& version, WindowManager& window, capo::IEngine& audio_engine)
		: finder(argv), text{finder}, data(*this), version(&version), window(&window), assets{finder}, sounds{finder}, music_player{audio_engine}, ambience_player{audio_engine}, quest_registry{finder}, quest_table{quest_registry},
		  soundboard{*this, audio_engine} {};

	util::Stopwatch stopwatch{};
	ResourceFinder finder;
	data::TextManager text;
	data::DataManager data;
	Version* version;
	WindowManager* window;
	core::SoundManager sounds;
	core::AssetManager assets;
	input::InputSystem input_system{finder};
	EventRegistry events;
	util::BitFlags<DebugFlags> debug_flags{};
	util::BitFlags<AppFlags> app_flags{};
	util::BitFlags<StateFlags> state_flags{};
	util::Ticker ticker{};
	WorldClock world_clock{};
	WorldTimer world_timer{*this};
	StateController state_controller{};
	MenuController menu_controller{};
	audio::Soundboard soundboard;
	audio::MusicPlayer music_player;
	audio::Ambience ambience_player;
	quest::QuestTracker quest{};
	NotificationManager notifications{};
	QuestRegistry quest_registry;
	QuestTable quest_table;
	StatTracker stats{};
	util::Logger logger{};
	config::AccessibilityService a11y{};
	graphics::CameraController camera_controller{};
	EditorSettings editor_settings{};
	int current_room{};

	// debug stuff
	int out_value{};

	void toggle_fullscreen() { fullscreen() ? app_flags.reset(AppFlags::fullscreen) : app_flags.set(AppFlags::fullscreen); }
	void toggle_tutorial() { tutorial() ? app_flags.reset(AppFlags::tutorial) : app_flags.set(AppFlags::tutorial); }
	void toggle_debug() { debug_mode() ? debug_flags.reset(DebugFlags::debug_mode) : debug_flags.set(DebugFlags::debug_mode); }
	void toggle_greyblock_mode() { greyblock_mode() ? debug_flags.reset(automa::DebugFlags::greyblock_mode) : debug_flags.set(automa::DebugFlags::greyblock_mode); }
	void set_fullscreen(bool flag) { flag ? app_flags.set(AppFlags::fullscreen) : app_flags.reset(AppFlags::fullscreen); }
	void set_editor(bool flag) { flag ? app_flags.set(AppFlags::editor) : app_flags.reset(AppFlags::editor); }
	void set_tutorial(bool flag) { flag ? app_flags.set(AppFlags::tutorial) : app_flags.reset(AppFlags::tutorial); }

	[[nodiscard]] auto fullscreen() const -> bool { return app_flags.test(AppFlags::fullscreen); }
	[[nodiscard]] auto is_editor() const -> bool { return app_flags.test(AppFlags::editor); }
	[[nodiscard]] auto tutorial() const -> bool { return app_flags.test(AppFlags::tutorial); }
	[[nodiscard]] auto in_game() const -> bool { return app_flags.test(AppFlags::in_game); }
	[[nodiscard]] auto hide_hud() const -> bool { return state_flags.test(StateFlags::hide_hud); }
	[[nodiscard]] auto no_menu() const -> bool { return state_flags.test(StateFlags::no_menu); }
	[[nodiscard]] auto demo_mode() const -> bool { return debug_flags.test(DebugFlags::demo_mode); }
	[[nodiscard]] auto greyblock_mode() const -> bool { return debug_flags.test(DebugFlags::greyblock_mode); }
	[[nodiscard]] auto debug_mode() const -> bool { return debug_flags.test(DebugFlags::debug_mode); }
	[[nodiscard]] auto in_window(sf::Vector2f point, sf::Vector2f dimensions) const -> bool { return window->in_window(point, dimensions); }
};
} // namespace fornani::automa
