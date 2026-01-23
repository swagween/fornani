
#pragma once

#include <capo/engine.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/setup/ResourceFinder.hpp>

namespace fornani::core {

class SoundManager {
  public:
	explicit SoundManager(ResourceFinder const& finder);
	capo::Buffer const& get_buffer(std::string const& label);

  private:
	std::unordered_map<std::string, capo::Buffer> m_buffers{};
	capo::Buffer m_null_buffer;
	fornani::io::Logger m_logger{"Core"};
};

} // namespace fornani::core
