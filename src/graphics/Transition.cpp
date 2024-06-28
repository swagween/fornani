
#include "Transition.hpp"
#include "../service/ServiceProvider.hpp"
#include "../entities/player/Player.hpp"

namespace flfx {

Transition::Transition(automa::ServiceProvider& svc, int d) : duration(d) {
	box.setPosition(0, 0);
	box.setSize(sf::Vector2<float>(static_cast<float>(svc.constants.screen_dimensions.x), static_cast<float>(svc.constants.screen_dimensions.y)));
	current_frame = 0;
}

void Transition::update(player::Player& player) {
	if (fade_in || fade_out) { player.controller.restrict(); }
	if (fade_out) {
		if (current_frame > 0) { current_frame -= rate; }
		if (alpha < 255) { alpha += rate; }
		if (current_frame <= 0) {
			fade_out = false;
			done = true;
		}
	} else if (fade_in) {
		done = false;
		if (current_frame < duration) { current_frame += rate; }
		if (alpha > 0) { alpha -= rate; }
		if (current_frame >= duration) { fade_in = false; }
	}
	if (done) {
		alpha = 255;
	} else if (!(fade_in || fade_out)) {
		alpha = 0;
		player.controller.unrestrict();
	}
}

void Transition::render(sf::RenderWindow& win) {
	if (fade_out || fade_in || done) {
		box.setFillColor(sf::Color{2, 1, 2, alpha});
		win.draw(box);
	}
}

} // namespace flfx
