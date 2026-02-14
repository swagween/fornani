
#include "fornani/world/Checkpoint.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/particle/Effect.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::world {

Checkpoint::Checkpoint(automa::ServiceProvider& svc, sf::Vector2f pos) : bounds({32.f, 32.f}) { bounds.set_position(pos); }

void Checkpoint::update(automa::ServiceProvider& svc, Map& map, player::Player& player) {
	if (player.get_collider().bounding_box.overlaps(bounds)) {
		for (auto& checkpoint : map.checkpoints) { checkpoint.unflag(); }
		flags.set(CheckpointState::reached);
	}
}

void Checkpoint::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (svc.greyblock_mode()) {
		bounds.render(win, cam);
		if (reached()) {
			sf::RectangleShape tag{};
			tag.setSize({32.f, 32.f});
			tag.setPosition(bounds.get_position() - cam);
			tag.setOutlineColor(colors::mythic_green);
			tag.setFillColor(sf::Color::Transparent);
			tag.setOutlineThickness(-2);
			win.draw(tag);
		}
	}
}

} // namespace fornani::world
