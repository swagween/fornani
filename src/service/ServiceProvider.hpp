#pragma once
#include "../utils/BitFlags.hpp"
#include "../setup/AssetManager.hpp"
#include "../setup/DataManager.hpp"
#include "../setup/TextManager.hpp"
#include "../automa/StateController.hpp"
#include "../graphics/Style.hpp"
#include "../utils/Random.hpp"
#include "../utils/Ticker.hpp"

namespace automa {
enum class DebugFlags { imgui_overlay, greyblock_mode };
struct ServiceProvider {
	asset::AssetManager assets{};
	data::DataManager data{};
	data::TextManager text{};
	style::Style styles{};
	util::BitFlags<DebugFlags> debug_flags{};
	util::Random random{};
	StateController state_controller{};
	util::Ticker ticker{};
};
} // namespace automa