
#include <fornani/setup/localization/Localization.hpp>

namespace fornani {

Localization::Localization(ResourceFinder& finder) {
	auto localization_result = dj::Json::from_file(finder.resource_path() + "/data/localization/localization.json");
	if (!localization_result) { NANI_LOG_ERROR(m_logger, "Failed to load localization data from {}.", finder.resource_path() + "/data/localization/localization.json"); }
	m_language_list = std::move(*localization_result);
	auto i = 0;
	for (auto const& [key, data] : m_language_list.as_object()) { m_available_languages.push_back(Language{key, data["title"].as_string(), i++}); }
}

void Localization::set_language(std::string_view code) {
	NANI_LOG_INFO(m_logger, "Attempting to set language to {}", code.data());
	for (auto const& lang : m_available_languages) {
		if (lang.code == code) {
			m_current_language = lang.index;
			NANI_LOG_INFO(m_logger, "Set language to {}", lang.code);
		}
	}
}

std::vector<std::string> Localization::get_copy_of_available_languages() {
	auto ret = std::vector<std::string>();
	for (auto const& entry : m_available_languages) { ret.push_back(entry.code); }
	return ret;
}

std::string_view Localization::get_tag_from_index(int index) {
	for (auto const& lang : m_available_languages) {
		if (lang.index == index) { return lang.code; }
	}
	return {};
}

} // namespace fornani
