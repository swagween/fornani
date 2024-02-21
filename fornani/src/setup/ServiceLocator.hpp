//
//  AssetManager.hpp
//

#pragma once

#include <stdio.h>
#include <SFML/Graphics.hpp>
#include <string>

#include "../automa/StateController.hpp"
#include "../entities/player/Player.hpp"
#include "../gui/Console.hpp"
#include "../service/Service.hpp"
#include "../utils/Camera.hpp"
#include "../utils/Clock.hpp"
#include "../utils/Ticker.hpp"
#include "../utils/InputState.hpp"
#include "../utils/Random.hpp"
#include "AssetManager.hpp"
#include "DataManager.hpp"

namespace svc {

enum class global_flags { greyblock_state, greyblock_trigger };
enum counters { draw_calls = 0 };
int const num_counters = 1;

inline auto assetLocator = forloop::Service<asset::AssetManager>::Instance{};
inline auto dataLocator = forloop::Service<data::DataManager>::Instance{};
inline auto playerLocator = forloop::Service<player::Player>::Instance{};
inline auto cameraLocator = forloop::Service<cam::Camera>::Instance{};
inline auto clockLocator = forloop::Service<util::Clock>::Instance{};
inline auto tickerLocator = forloop::Service<util::Ticker>::Instance{};
inline auto stateControllerLocator = forloop::Service<automa::StateController>::Instance{};
inline auto consoleLocator = forloop::Service<gui::Console>::Instance{};
inline auto randomLocator = forloop::Service<util::Random>::Instance{};
inline auto globalBitFlagsLocator = forloop::Service<util::BitFlags<global_flags>>::Instance{};
inline auto inputStateLocator = forloop::Service<util::InputState>::Instance{};
inline auto counterLocator = forloop::Service<std::array<uint16_t, num_counters>>::Instance{};
inline auto floatReadoutLocator = forloop::Service<float>::Instance{};

} // namespace svc
