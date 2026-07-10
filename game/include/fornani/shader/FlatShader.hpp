
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/io/Logger.hpp>

namespace fornani {

class FlatShader {
  public:
	FlatShader(ResourceFinder& finder);
	void finalize(sf::Color color);
	void submit(sf::RenderWindow& win, sf::Sprite const& sprite);

  private:
	sf::Shader m_shader{};
	io::Logger m_logger{"shader"};
};

} // namespace fornani
