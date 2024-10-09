#include "MiniMenu.hpp"
#include "../service/ServiceProvider.hpp"
#include "../automa/GameState.hpp"
#include <algorithm>

namespace gui {

MiniMenu::MiniMenu(automa::ServiceProvider& svc, std::vector<std::string_view> opt, bool white) {
	white ? sprite.set_texture(svc.assets.t_cream_console) : sprite.set_texture(svc.assets.t_blue_console);
	sprite.slice(svc, static_cast<int>(corner), static_cast<int>(edge));
	auto ctr{0};
	for (auto& o : opt) {
		options.push_back(automa::Option(svc, o, svc.text.fonts.title, white));
		options.back().index = ctr;
		options.back().update(svc, selection.get());
		++ctr;
	}
	selection = util::Circuit(static_cast<int>(options.size()));
}

void MiniMenu::overwrite_option(int index, std::string_view replacement) {
	if (index >= options.size()) { return; }
		options.at(index).set_string(replacement);
	}

void MiniMenu::update(automa::ServiceProvider& svc, sf::Vector2<float> dim, sf::Vector2<float> position) {
	if (!is_open()) { return; }
	dimensions = dim;
	sprite.update(svc, position, dim, corner, edge);
	auto spacing = 12.f;
	auto top_buffer = 18.f;
	auto ctr{0};
	for (auto& option : options) {
		option.position = {sprite.get_position().x, sprite.get_position().y + spacing + ctr * (option.label.getLocalBounds().height + spacing) - sprite.get_center().y + top_buffer};
		option.update(svc, selection.get());
		++ctr;
	}
}

void MiniMenu::render(sf::RenderWindow& win, bool bg) const {
	if (!is_open()) { return; }
	if (bg) { sprite.render(win); }
	if (!sprite.is_extended()) { return; }
	for (auto& option : options) { win.draw(option.label); }
}

void MiniMenu::open(automa::ServiceProvider& svc, sf::Vector2<float> position) {
	state.set(MiniMenuState::open);
	sprite.start(svc, position);
}

void MiniMenu::close(automa::ServiceProvider& svc) {
	state.reset(MiniMenuState::open);
	sprite.start(svc, position);
	selection.zero();
}

void MiniMenu::set_origin(sf::Vector2<float> origin) { sprite.set_origin(origin); }

void MiniMenu::up(automa::ServiceProvider& svc) {
	selection.modulate(-1);
	svc.soundboard.flags.console.set(audio::Console::shift);
}

void MiniMenu::down(automa::ServiceProvider& svc) {
	selection.modulate(1);
	svc.soundboard.flags.console.set(audio::Console::shift);
}

sf::Vector2<float> MiniMenu::get_dimensions() const { return dimensions; }

} // namespace gui
