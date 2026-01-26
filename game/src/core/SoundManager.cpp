
#include "fornani/core/SoundManager.hpp"

namespace fornani::core {

SoundManager::SoundManager(ResourceFinder const& finder) {
	namespace fs = std::filesystem;
	auto sfx_dir = fs::path{finder.resource_path()} / "audio" / "sfx";
	auto file = sfx_dir / fs::path{"error_sound.wav"};
	auto file_str = file.string();

	// null buffer for lookup failures
	if (!m_null_buffer.decode_file(file_str.c_str(), capo::Encoding::Wav)) { NANI_LOG_WARN(m_logger, "Failed to load null buffer."); };

	// recursively load sound effects
	for (auto const& sfx_genre : fs::recursive_directory_iterator(sfx_dir)) {
		if (!sfx_genre.is_directory()) { continue; }
		if (sfx_genre.path().filename().string() == "unused") { continue; }
		for (auto const& sfx : fs::recursive_directory_iterator(sfx_genre)) {
			if (sfx.path().extension() != ".wav") { continue; }
			auto sfx_str = sfx.path().filename().string();
			auto in_buffer = capo::Buffer{};
			auto pathstr = sfx.path().string();
			if (!in_buffer.decode_file(pathstr.c_str(), capo::Encoding::Wav)) { NANI_LOG_ERROR(m_logger, "Failed to decode file [{}]", pathstr); }
			auto filename = sfx_str.substr(0, sfx_str.find('.'));
			m_buffers.insert({filename, in_buffer});
			m_ticks.insert({filename, 0});
		}
	}
}

capo::Buffer const& SoundManager::get_buffer(std::string_view label) {
	auto it = m_buffers.find(label);
	if (it == m_buffers.end()) { return m_null_buffer; }
	return it->second;
}

void SoundManager::set_tick_for_buffer(std::string_view label, int const to) {
	auto it = m_ticks.find(label);
	if (it == m_ticks.end()) { return; }
	it->second = to;
}

int SoundManager::get_tick_for_buffer(std::string_view label) {
	auto it = m_ticks.find(label);
	if (it == m_ticks.end()) { return -1; }
	return it->second;
}

} // namespace fornani::core
