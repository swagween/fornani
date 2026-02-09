
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/automa/Option.hpp>
#include <fornani/core/Common.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <fornani/utils/Circuit.hpp>
#include <fornani/utils/NineSlice.hpp>
#include <string>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::input {
class InputSystem;
}

namespace fornani::audio {
class Soundboard;
}

namespace fornani::gui {

enum class MiniMenuAttributes { no_ease };
enum class MiniMenuFlags { selected, closed };

class MiniMenu {
  public:
	MiniMenu(automa::ServiceProvider& svc, std::vector<std::string> opt, sf::Vector2f start_position, automa::MenuTheme& theme);
	void update(automa::ServiceProvider& svc, sf::Vector2f dim, sf::Vector2f at_position);
	void render(sf::RenderWindow& win, bool bg = true);
	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void handle_inputs(input::InputSystem& controller, [[maybe_unused]] audio::Soundboard& soundboard);
	void set_dimensions(sf::Vector2f to) { m_nineslice.set_dimensions(to); }
	void set_attribute(MiniMenuAttributes to, bool flag = true) { flag ? m_attributes.set(to) : m_attributes.reset(to); }

	sf::Vector2f get_dimensions() const;

	[[nodiscard]] auto get_selection() const -> int { return selection.get(); }
	[[nodiscard]] auto was_selected() const -> int { return m_flags.test(MiniMenuFlags::selected); }
	[[nodiscard]] auto was_closed() const -> int { return m_flags.test(MiniMenuFlags::closed); }
	[[nodiscard]] auto was_last_option() const -> int { return selection.get() == options.size() - 1; }
	[[nodiscard]] auto get_option() const -> std::string { return options.size() > selection.get() ? options.at(selection.get()).label.getString() : null_key; }

	sf::Vector2f position{};
	sf::Vector2f draw_position{};

  private:
	automa::MenuTheme* m_theme;
	util::BitFlags<MiniMenuAttributes> m_attributes{};
	util::BitFlags<MiniMenuFlags> m_flags{};
	sf::Vector2f dimensions{};
	int maximum{};
	int index{};
	util::NineSlice m_nineslice;
	util::Circuit selection{1};
	std::vector<automa::Option> options;

	io::Logger m_logger{"GUI"};
};

} // namespace fornani::gui
