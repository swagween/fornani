#include "Indicator.hpp"
#include "Indicator.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../../gui/Console.hpp"
#include "Player.hpp"

namespace player {

Indicator::Indicator(automa::ServiceProvider& svc) {
	font.loadFromFile(svc.text.title_font);
	font.setSmooth(false);
	label.setCharacterSize(16);
	label.setFont(font);
	label.setColor(svc.styles.colors.ui_white);
}

void Indicator::update(player::Player& player) {
	addition_limit.update();
	label.setPosition(player.collider.physics.position - offset);
}

void Indicator::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (svc.greyblock_mode()) {
		return;
	} else if (!addition_limit.is_complete()) {
		label.setPosition(label.getPosition() - cam);
		win.draw(label);
	}
}

void Indicator::add(int amount) {
	variables.amount += amount;
	label.setString("+" + std::to_string(variables.amount));
	addition_limit.start(addition_time);
}

} // namespace player
