#pragma once
#include <SFML/Graphics.hpp>
#include <string_view>
#include "fornani/automa/Option.hpp"
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Circuit.hpp"
#include "fornani/utils/NineSlice.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::gui {

class MiniMenu {
  public:
	MiniMenu(automa::ServiceProvider& svc, std::vector<std::string_view> opt, sf::Vector2f start_position, bool white = false);
	void update(automa::ServiceProvider& svc, sf::Vector2f dim, sf::Vector2f at_position);
	void render(sf::RenderWindow& win, bool bg = true);
	void up(automa::ServiceProvider& svc);
	void down(automa::ServiceProvider& svc);
	sf::Vector2f get_dimensions() const;
	[[nodiscard]] auto get_selection() const -> int { return selection.get(); }
	sf::Vector2f position{};
	sf::Vector2f draw_position{};

  private:
	sf::Vector2f dimensions{};
	int maximum{};
	int index{};
	util::NineSlice m_nineslice;
	util::Circuit selection{1};
	std::vector<automa::Option> options;
};

} // namespace fornani::gui
