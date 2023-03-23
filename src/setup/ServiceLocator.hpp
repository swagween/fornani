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

namespace svc {

inline auto assetLocator = forloop::Service<AssetManager>::Instance{};
inline auto playerLocator = forloop::Service<Player>::Instance{};
inline auto cameraLocator = forloop::Service<Camera>::Instance{};
inline auto clockLocator = forloop::Service<util::Clock>::Instance{};

}

/* ServiceLocator_hpp */
