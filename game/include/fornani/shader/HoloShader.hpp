
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/io/Logger.hpp>

namespace fornani {

class HoloShader {
  public:
	HoloShader(ResourceFinder& finder);
	void finalize(float time, sf::Color highlight, sf::Color shadow, float glow = 1.1f);
	void submit(sf::RenderWindow& win, sf::Sprite const& sprite);

  private:
	sf::Shader m_shader{};
	io::Logger m_logger{"shader"};
};

} // namespace fornani
