
#pragma once

#include <djson/json.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/setup/ResourceFinder.hpp>
#include <optional>
#include <string_view>

namespace fornani {

struct Language {
	Language(dj::Json const& in);
	std::string code{};
	std::string title{};
};

class Localization {
  public:
	Localization(ResourceFinder& finder);
	void set_language(std::string_view code);

	[[nodiscard]] auto get_language_code() const -> std::optional<std::string_view> {
		return m_current_language.transform([](auto const& lang) { return std::string_view{lang.code}; });
	}

	[[nodiscard]] auto get_language_title() const -> std::optional<std::string_view> {
		return m_current_language.transform([](auto const& lang) { return std::string_view{lang.title}; });
	}

	[[nodiscard]] auto get_folder_string() const -> std::string {
		if (auto code = get_language_code()) { return "/localization/" + std::string{code.value().data()}; }
		return "/localization/eng"; // default to english in case language was not found
	}

  private:
	std::optional<Language> m_current_language{};
	dj::Json m_language_list{};

	io::Logger m_logger{"Localization"};
};

} // namespace fornani
