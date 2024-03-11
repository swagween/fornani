#pragma once
#include "../utils/BitFlags.hpp"
#include "../setup/AssetManager.hpp"
#include "../setup/TextManager.hpp"
#include "../automa/StateController.hpp"

namespace automa {
enum class DebugFlags { imgui_overlay, greyblock_mode };
struct ServiceProvider {
	asset::AssetManager assets{}; //unused
	data::TextManager text{};
	util::BitFlags<DebugFlags> debug_flags{};
	StateController state_controller{};
};
} // namespace automa