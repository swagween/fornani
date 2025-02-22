#include "fornani/gui/MiniMenu.hpp"
#include <algorithm>
#include "fornani/automa/GameState.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::gui {

MiniMenu::MiniMenu(automa::ServiceProvider& svc, std::vector<std::string_view> opt, sf::Vector2f start_position, bool white)
	: m_nineslice{svc, (white ? svc.assets.get_texture("cream_console") : svc.assets.get_texture("blue_console")), {static_cast<int>(corner), static_cast<int>(corner)}, {static_cast<int>(edge), static_cast<int>(edge)}} {
	;
	auto ctr{0};
	for (auto& o : opt) {
		options.push_back(automa::Option(svc, o, white));
		options.back().index = ctr;
		options.back().update(svc, selection.get());
		++ctr;
	}
	selection = util::Circuit(static_cast<int>(options.size()));
}

void MiniMenu::update(automa::ServiceProvider& svc, sf::Vector2<float> dim, sf::Vector2<float> at_position) {
	dimensions = dim;
	m_nineslice.set_position(at_position);
	m_nineslice.set_dimensions(dim);
	auto spacing = 12.f;
	auto top_buffer = 18.f;
	auto ctr{0};
	for (auto& option : options) {
		option.position = {m_nineslice.get_position().x, m_nineslice.get_position().y + spacing + ctr * (option.label.getLocalBounds().size.y + spacing) - m_nineslice.get_local_center().y + top_buffer};
		option.update(svc, selection.get());
		++ctr;
	}
}

void MiniMenu::render(sf::RenderWindow& win, bool bg) {
	if (bg) { m_nineslice.render(win); }
	for (auto& option : options) { win.draw(option.label); }
}

void MiniMenu::up(automa::ServiceProvider& svc) {
	selection.modulate(-1);
	svc.soundboard.flags.console.set(audio::Console::shift);
}

void MiniMenu::down(automa::ServiceProvider& svc) {
	selection.modulate(1);
	svc.soundboard.flags.console.set(audio::Console::shift);
}

sf::Vector2<float> MiniMenu::get_dimensions() const { return dimensions; }

} // namespace fornani::gui
