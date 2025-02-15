#include "fornani/level/Checkpoint.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/level/Map.hpp"
#include "fornani/particle/Effect.hpp"
#include "fornani/entities/player/Player.hpp"

namespace fornani::world {

Checkpoint::Checkpoint(automa::ServiceProvider& svc, sf::Vector2<float> pos) : bounds({32.f, 32.f}) { bounds.set_position(pos); }

void Checkpoint::update(automa::ServiceProvider& svc, Map& map, player::Player& player) {
	if (player.collider.bounding_box.overlaps(bounds)) {
		for (auto& checkpoint : map.checkpoints) { checkpoint.unflag(); }
		flags.set(CheckpointState::reached);
	}
}

void Checkpoint::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (svc.greyblock_mode()) {
		bounds.render(win, cam);
		if (reached()) {
			sf::RectangleShape tag{};
			tag.setSize({32.f, 32.f});
			tag.setPosition(bounds.get_position() - cam);
			tag.setOutlineColor(svc.styles.colors.mythic_green);
			tag.setFillColor(sf::Color::Transparent);
			tag.setOutlineThickness(-2);
			win.draw(tag);
		}
	}
}

} // namespace world
