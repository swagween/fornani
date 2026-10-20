
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/io/Logger.hpp>

namespace fornani {

enum class LightBlendMode : int { normal = 0, additive = 1, screen = 2 };

class FlatShader {

  public:
	FlatShader(ResourceFinder& finder);
	void finalize(sf::Color color);
	void submit(sf::RenderWindow& win, sf::Sprite const& sprite, LightBlendMode const blend_mode = LightBlendMode::normal);

  private:
	sf::Shader m_shader{};
	io::Logger m_logger{"shader"};
};

} // namespace fornani
