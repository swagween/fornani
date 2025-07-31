
#pragma once

#include "fornani/io/Logger.hpp"
#include "fornani/setup/EnumLookups.hpp"
#include "fornani/setup/ResourceFinder.hpp"

#include <SFML/Graphics.hpp>

namespace fornani::core {

class AssetManager {
  public:
	explicit AssetManager(ResourceFinder const& finder);

	sf::Texture const& get_texture(std::string const& label);
	sf::Texture& get_texture_modifiable(std::string const& label);
	sf::Texture const& get_tileset(std::string const& label);
	sf::Texture const& get_npc_texture(std::string const& label);

  private:
	sf::Texture m_null_texture{};
	std::unordered_map<std::string, sf::Texture> m_textures{};
	fornani::io::Logger m_logger{"core"};
};

} // namespace fornani::core
