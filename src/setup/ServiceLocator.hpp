//
//  AssetManager.hpp
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once


#include <SFML/Graphics.hpp>
#include <stdio.h>
#include <string>

#include "AssetManager.hpp"
#include "../entities/player/Player.hpp"
#include "../utils/Camera.hpp"
#include "../service/Service.hpp"
#include "../utils/Clock.hpp"
#include "../automa/StateController.hpp"
#include "../gui/Console.hpp"
#include "../utils/Random.hpp"

namespace svc {

	enum class bit_state {state, trigger};

	inline auto assetLocator = forloop::Service<AssetManager>::Instance{};
	inline auto playerLocator = forloop::Service<Player>::Instance{};
	inline auto cameraLocator = forloop::Service<cam::Camera>::Instance{};
	inline auto clockLocator = forloop::Service<util::Clock>::Instance{};
	inline auto stateControllerLocator = forloop::Service<automa::StateController>::Instance{};
	inline auto consoleLocator = forloop::Service<gui::Console>::Instance{};
	inline auto randomLocator = forloop::Service<util::Random>::Instance{};
	inline auto greyboxModeLocator = forloop::Service < util::BitFlags<bit_state> > ::Instance{};

}

/* ServiceLocator_hpp */
