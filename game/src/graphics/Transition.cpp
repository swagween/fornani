
#include "fornani/graphics/Transition.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/entities/player/Player.hpp"

namespace flfx {

Transition::Transition(automa::ServiceProvider& svc, int d) : duration(d) {
	cooldown = util::Cooldown{d};
	color = svc.styles.colors.ui_black;
	box.setSize(sf::Vector2<float>(static_cast<float>(svc.constants.screen_dimensions.x), static_cast<float>(svc.constants.screen_dimensions.y)));
}

void Transition::update(player::Player& player) {
	auto tt = static_cast<float>(duration);
	if ((fade_in || fade_out) && !player.controller.walking_autonomously()) { player.controller.restrict_movement(); }
	if (fade_out) {
		auto timer = (tt - static_cast<float>(cooldown.get_cooldown())) / tt;
		alpha = static_cast<uint8_t>(std::lerp(0, 255, timer));
		if (cooldown.is_complete()) {
			fade_out = false;
			done = true;
		}
	} else if (fade_in) {
		auto timer = static_cast<float>(cooldown.get_cooldown()) / tt;
		alpha = static_cast<uint8_t>(std::lerp(0, 255, timer));
		if (cooldown.is_complete()) { fade_in = false; }
	}
	if (done) {
		alpha = 255;
	} else if (!(fade_in || fade_out)) {
		alpha = 0;
		player.controller.unrestrict();
	}
	cooldown.update();
}

void Transition::render(sf::RenderWindow& win) {
	if (fade_out || fade_in || done) {
		color.a = alpha;
		box.setFillColor(color);
		win.draw(box);
	}
}

void Transition::start() {
	if (!fade_out) { cooldown.start(); }
	fade_in = false;
	fade_out = true;
}

void Transition::end() {
	if (!fade_in) { cooldown.start(); }
	done = false;
	fade_in = true;
	fade_out = false;
}

} // namespace flfx
