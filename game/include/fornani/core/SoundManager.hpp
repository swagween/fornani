
#pragma once

#include "fornani/io/Logger.hpp"
#include "fornani/setup/EnumLookups.hpp"
#include "fornani/setup/ResourceFinder.hpp"

#include <capo/engine.hpp>

namespace fornani::core {

class SoundManager {
  public:
	explicit SoundManager(data::ResourceFinder const& finder);
	capo::Buffer const& get_buffer(std::string const& label);

  private:
	std::unordered_map<std::string, capo::Buffer> m_buffers{};
	capo::Buffer m_null_buffer;
	fornani::io::Logger m_logger{"core"};
};

} // namespace fornani::core
