#include "MiniMenu.hpp"
#include "../service/ServiceProvider.hpp"
#include "../automa/GameState.hpp"
#include <algorithm>

namespace gui {

MiniMenu::MiniMenu(automa::ServiceProvider& svc, std::vector<std::string_view> opt) {
	sprite.set_texture(svc.assets.t_white_console);
	sprite.slice(corner, edge);
	for (auto& o : opt) { options.push_back(automa::Option(svc, o)); }
	selection = util::Circuit(static_cast<int>(options.size()));
}

void MiniMenu::update(automa::ServiceProvider& svc, sf::Vector2<float> dim, sf::Vector2<float> position) {
	dimensions = dim;
	sprite.update(position, dim, corner, edge);
	for (auto& option : options) { option.update(svc, selection.get()); }
}

void MiniMenu::render(sf::RenderWindow& win) const {
	if (!is_open()) { return; }
	sprite.render(win);
	for (auto& option : options) { win.draw(option.label); }
}

void MiniMenu::open(sf::Vector2<float> position) {
	state.set(MiniMenuState::open);
	sprite.start();
}

void MiniMenu::set_origin(sf::Vector2<float> origin) { sprite.set_origin(origin); }

void MiniMenu::up() { selection.modulate(-1); }

void MiniMenu::down() { selection.modulate(1); }

sf::Vector2<float> MiniMenu::get_dimensions() const { return dimensions; }

} // namespace gui
