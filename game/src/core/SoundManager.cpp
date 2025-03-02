
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
	m_buffers.insert({"pioneer_open", sf::SoundBuffer{finder.resource_path() / p_gui / fs::path{"pioneer_open.wav"}}});
	m_buffers.insert({"pioneer_close", sf::SoundBuffer{finder.resource_path() / p_gui / fs::path{"pioneer_close.wav"}}});
	m_buffers.insert({"pioneer_slot", sf::SoundBuffer{finder.resource_path() / p_gui / fs::path{"pioneer_slot.wav"}}});
	m_buffers.insert({"pioneer_chain", sf::SoundBuffer{finder.resource_path() / p_gui / fs::path{"pioneer_chain.wav"}}});
	m_buffers.insert({"pioneer_buzz", sf::SoundBuffer{finder.resource_path() / p_gui / fs::path{"pioneer_buzz.wav"}}});
	m_buffers.insert({"pioneer_hum", sf::SoundBuffer{finder.resource_path() / p_gui / fs::path{"pioneer_hum.wav"}}});
	m_buffers.insert({"pioneer_hard_slot", sf::SoundBuffer{finder.resource_path() / p_gui / fs::path{"pioneer_hard_slot.wav"}}});
	m_buffers.insert({"pioneer_fast_click", sf::SoundBuffer{finder.resource_path() / p_gui / fs::path{"pioneer_fast_click.wav"}}});
	m_buffers.insert({"pioneer_sync", sf::SoundBuffer{finder.resource_path() / p_gui / fs::path{"pioneer_sync.wav"}}});
	m_buffers.insert({"pioneer_boot", sf::SoundBuffer{finder.resource_path() / p_gui / fs::path{"pioneer_boot.wav"}}});
	m_buffers.insert({"pioneer_scan", sf::SoundBuffer{finder.resource_path() / p_gui / fs::path{"pioneer_scan.wav"}}});
}

sf::SoundBuffer const& SoundManager::get_buffer(std::string const& label) { return m_buffers.contains(label) ? m_buffers.at(label) : m_null_buffer; }

} // namespace fornani::core
