
#pragma once

#include <SFML/Graphics.hpp>

namespace fornani {

struct LayerTexture {
	sf::RenderTexture day{};
	sf::RenderTexture twilight{};
	sf::RenderTexture night{};
};

} // namespace fornani
