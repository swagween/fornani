
#pragma once

#include "fornani/io/Logger.hpp"
#include "fornani/setup/EnumLookups.hpp"
#include "fornani/setup/ResourceFinder.hpp"

#include <SFML/Audio.hpp>

namespace fornani::core {

class SoundManager {
  public:
	explicit SoundManager(data::ResourceFinder const& finder);
	sf::SoundBuffer const& get_buffer(std::string const& label);

  private:
	std::unordered_map<std::string, sf::SoundBuffer> m_buffers{};
	sf::SoundBuffer m_null_buffer;
	fornani::io::Logger m_logger{"core"};
};

} // namespace fornani::core
