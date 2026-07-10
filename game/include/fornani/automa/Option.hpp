
#pragma once

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <string_view>

namespace fornani {
class MenuTheme;
}

namespace fornani::automa {

enum class TextJustification { left, center, right };

struct ServiceProvider;
struct Option {
	Option(ServiceProvider& svc, MenuTheme& theme, std::string_view lbl);
	void set_string(std::string_view str);
	sf::Text label;
	sf::Vector2f position{};
	int index{};
	bool highlighted{};
	bool selected{};
	bool flagged{};
	bool selectable;
	void update(int selection, TextJustification justification = TextJustification::center);

  private:
	MenuTheme* m_theme;
	int m_last_selection{};
	util::Cooldown m_fade;
};

} // namespace fornani::automa
