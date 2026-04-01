
#include <fornani/setup/localization/Localization.hpp>

namespace fornani {

Localization::Localization(ResourceFinder& finder) {
	auto localization_result = dj::Json::from_file(finder.resource_path() + "/data/config/localization.json");
	if (!localization_result) { NANI_LOG_ERROR(m_logger, "Failed to load localization data from {}.", finder.resource_path() + "/data/config/localization.json"); }
	m_language_list = std::move(*localization_result);
}

void Localization::set_language(std::string_view code) {
	NANI_LOG_INFO(m_logger, "Attempting to set language to {}", code.data());
	m_current_language = Language(m_language_list[code]);
	m_current_language->code = code.data();
	NANI_LOG_INFO(m_logger, "Set language to {}", m_current_language->code);
}

Language::Language(dj::Json const& in) { title = in["title"].as_string(); }

} // namespace fornani
