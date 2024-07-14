#pragma once
#include "../utils/BitFlags.hpp"
#include "../setup/AssetManager.hpp"
#include "../setup/DataManager.hpp"
#include "../setup/TextManager.hpp"
#include "../setup/Tables.hpp"
#include "../automa/StateController.hpp"
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
struct ServiceProvider {
	asset::AssetManager assets{};
	data::DataManager data{*this};
	data::TextManager text{};
	config::ControllerMap controller_map{*this};
	style::Style styles{};
	util::BitFlags<DebugFlags> debug_flags{};
	util::Random random{};
	util::Ticker ticker{};
	util::Constants constants{};
	lookup::Tables tables{};
	StateController state_controller{};
	audio::Soundboard soundboard{};
	audio::MusicPlayer music{};
	fornani::QuestTracker quest{};
	fornani::StatTracker stats{};

	//debug stuff
	util::Stopwatch stopwatch{};

	[[nodiscard]] auto demo_mode() const -> bool { return debug_flags.test(DebugFlags::demo_mode); }
	[[nodiscard]] auto greyblock_mode() const -> bool { return debug_flags.test(DebugFlags::greyblock_mode); }
	[[nodiscard]] auto death_mode() const -> bool { return state_controller.actions.test(Actions::death_mode); }

};
} // namespace automa