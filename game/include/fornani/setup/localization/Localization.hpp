
#pragma once

#include <djson/json.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/setup/ResourceFinder.hpp>
#include <optional>
#include <string_view>

namespace fornani {

struct Language {
	std::string code{};
	std::string title{};
	int index{};
};

class Localization {
  public:
	Localization(ResourceFinder& finder);
	void set_language(std::string_view code);

	std::vector<Language> get_available_languages() const& { return m_available_languages; }
	std::vector<std::string> get_copy_of_available_languages();
	std::string_view get_tag_from_index(int index);

	[[nodiscard]] auto get_language_code() const -> std::string_view { return m_available_languages.at(m_current_language).code; }
	[[nodiscard]] auto get_language_title() const -> std::string_view { return m_available_languages.at(m_current_language).title; }
	[[nodiscard]] auto get_folder_string() const -> std::string { return "/localization/" + std::string{get_language_code()}; }

  private:
	std::size_t m_current_language{};
	dj::Json m_language_list{};
	std::vector<Language> m_available_languages{};

	io::Logger m_logger{"Localization"};
};

} // namespace fornani
