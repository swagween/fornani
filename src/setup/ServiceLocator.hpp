//
//  AssetManager.hpp
//

#pragma once

#include <stdio.h>
#include <SFML/Graphics.hpp>
#include <string>

#include "../automa/StateController.hpp"
#include "../service/Service.hpp"
#include "../utils/Stopwatch.hpp"
#include "../utils/Ticker.hpp"
#include "../utils/InputState.hpp"
#include "AssetManager.hpp"
#include "DataManager.hpp"
#include "TextManager.hpp"
#include "../audio/Soundboard.hpp"
#include "../audio/MusicPlayer.hpp"

namespace services {

enum class global_flags { greyblock_state, greyblock_trigger };
enum counters { draw_calls = 0 };
int const num_counters = 1;

class ServiceLocator {
  public:
	forloop::Service<asset::AssetManager>::Instance assetLocator{};
	forloop::Service<data::DataManager>::Instance dataLocator{};
	forloop::Service<data::TextManager>::Instance textLocator{};
	//forloop::Service<player::Player>::Instance playerLocator{};
	//forloop::Service<cam::Camera>::Instance cameraLocator{};
	forloop::Service<util::Stopwatch>::Instance stopwatchLocator{};
	forloop::Service<util::Ticker>::Instance tickerLocator{};
	forloop::Service<automa::StateController>::Instance stateControllerLocator{};
	//forloop::Service<gui::Console>::Instance consoleLocator{};
	//forloop::Service<util::Random>::Instance randomLocator{};
	forloop::Service<util::BitFlags<global_flags>>::Instance globalBitFlagsLocator{};
	forloop::Service<util::InputState>::Instance inputStateLocator{};
	forloop::Service<std::array<uint16_t, num_counters>>::Instance counterLocator{};
	//forloop::Service<float>::Instance floatReadoutLocator{};
	//forloop::Service<util::Logger>::Instance loggerLocator{};
	forloop::Service<audio::Soundboard>::Instance soundboardLocator{};
	forloop::Service<audio::MusicPlayer>::Instance musicPlayerLocator{};
};

} // namespace svc
