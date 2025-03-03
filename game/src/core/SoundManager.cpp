
#include "fornani/core/SoundManager.hpp"

namespace fornani::core {

SoundManager::SoundManager(data::ResourceFinder const& finder) {
	namespace fs = std::filesystem;
	auto sfx_dir = fs::path{finder.resource_path()} / "audio" / "sfx";

	// null buffer for lookup failures
	if (!m_null_buffer.loadFromFile(sfx_dir / fs::path{"error_sound.wav"})) { NANI_LOG_WARN(m_logger, "Failed to load null buffer."); };

	// recursively load sound effects
	for (auto const& sfx_genre : fs::recursive_directory_iterator(sfx_dir)) {
		if (!sfx_genre.is_directory()) { continue; }
		if (sfx_genre.path().filename().string() == "unused") { continue; }
		for (auto const& sfx : fs::recursive_directory_iterator(sfx_genre)) {
			if (sfx.path().extension() != ".wav") { continue; }
			auto sfx_str = sfx.path().filename().string();
			m_buffers.insert({sfx_str.substr(0, sfx_str.find('.')), sf::SoundBuffer{sfx.path()}});
		}
	}
}

sf::SoundBuffer const& SoundManager::get_buffer(std::string const& label) { return m_buffers.contains(label) ? m_buffers.at(label) : m_null_buffer; }

} // namespace fornani::core
