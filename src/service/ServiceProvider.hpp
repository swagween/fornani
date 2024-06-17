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

namespace automa {
enum class DebugFlags { imgui_overlay, greyblock_mode, greyblock_trigger };
struct ServiceProvider {
	asset::AssetManager assets{};
	data::DataManager data{*this};
	data::TextManager text{};
	config::ControllerMap controller_map{*this};
	audio::Soundboard soundboard{};
	audio::MusicPlayer music{};
	style::Style styles{};
	util::BitFlags<DebugFlags> debug_flags{};
	util::Random random{};
	util::Ticker ticker{};
	util::Constants constants{};
	lookup::Tables tables{};
	StateController state_controller{};

	//debug stuff
	util::Stopwatch stopwatch{};
	[[nodiscard]] auto greyblock_mode() const -> bool { return debug_flags.test(DebugFlags::greyblock_mode); }

};
} // namespace automa