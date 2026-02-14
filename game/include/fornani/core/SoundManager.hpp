
#pragma once

#include <capo/engine.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/setup/ResourceFinder.hpp>
#include <fornani/utils/TransparentStringHash.hpp>

namespace fornani::core {

class SoundManager {
  public:
	explicit SoundManager(ResourceFinder const& finder);
	capo::Buffer const& get_buffer(std::string_view label);
	void set_tick_for_buffer(std::string_view label, int const to);
	int get_tick_for_buffer(std::string_view label);

  private:
	std::unordered_map<std::string, capo::Buffer, TransparentHash, TransparentEqual> m_buffers{};
	std::unordered_map<std::string, int, TransparentHash, TransparentEqual> m_ticks{};
	capo::Buffer m_null_buffer;
	fornani::io::Logger m_logger{"Core"};
};

} // namespace fornani::core
