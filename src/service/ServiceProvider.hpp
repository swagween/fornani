#pragma once
#include "../utils/BitFlags.hpp"
#include "../setup/AssetManager.hpp"
#include "../setup/DataManager.hpp"
#include "../setup/TextManager.hpp"
#include "../automa/StateController.hpp"
#include "../graphics/Style.hpp"
#include "../utils/Random.hpp"
#include "../utils/Ticker.hpp"
#include "../utils/Constants.hpp"

namespace automa {
enum class DebugFlags { imgui_overlay, greyblock_mode, greyblock_trigger };
struct ServiceProvider {
	asset::AssetManager assets{};
	data::DataManager data{};
	data::TextManager text{};
	style::Style styles{};
	util::BitFlags<DebugFlags> debug_flags{};
	util::Random random{};
	util::Ticker ticker{};
	util::Constants constants{};
	StateController state_controller{};

	[[nodiscard]] auto greyblock_mode() const -> bool { return debug_flags.test(DebugFlags::greyblock_mode); }

};
} // namespace automa