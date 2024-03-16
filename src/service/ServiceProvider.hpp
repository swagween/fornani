#pragma once
#include "../utils/BitFlags.hpp"
#include "../setup/AssetManager.hpp"
#include "../setup/DataManager.hpp"
#include "../setup/TextManager.hpp"
#include "../automa/StateController.hpp"
#include "../graphics/Style.hpp"

namespace automa {
enum class DebugFlags { imgui_overlay, greyblock_mode };
struct ServiceProvider {
	asset::AssetManager assets{};
	data::DataManager data{};
	data::TextManager text{};
	style::Style styles{};
	util::BitFlags<DebugFlags> debug_flags{};
	StateController state_controller{};
};
} // namespace automa