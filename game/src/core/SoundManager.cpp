
#include "fornani/core/SoundManager.hpp"

namespace fornani::core {

SoundManager::SoundManager(data::ResourceFinder const& finder) {
	namespace fs = std::filesystem;
	auto p_folder{fs::path{"audio"}};
	auto p_sfx{p_folder / fs::path{"sfx"}};
	auto p_gui{p_sfx / fs::path{"gui"}};

	// null buffer for lookup failures
	if (!m_null_buffer.loadFromFile(finder.resource_path() / p_sfx / fs::path{"error_sound.wav"})) { NANI_LOG_WARN(m_logger, "Failed to load null buffer."); };

	// populate buffer map
	m_buffers.insert({"pioneer_select", sf::SoundBuffer{finder.resource_path() / p_gui / fs::path{"pioneer_select.wav"}}});
	m_buffers.insert({"pioneer_back", sf::SoundBuffer{finder.resource_path() / p_gui / fs::path{"pioneer_back.wav"}}});
	m_buffers.insert({"pioneer_click", sf::SoundBuffer{finder.resource_path() / p_gui / fs::path{"pioneer_click.wav"}}});
}

sf::SoundBuffer const& SoundManager::get_buffer(std::string const& label) { return m_buffers.contains(label) ? m_buffers.at(label) : m_null_buffer; }

} // namespace fornani::core
