#include "Bed.hpp"

#include "Bed.hpp"
#include "../../gui/Console.hpp"
#include "../../level/Map.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../player/Player.hpp"

namespace entity {

Bed::Bed(automa::ServiceProvider& svc, sf::Vector2<float> position, int room) : room(room) {
	sparkler = vfx::Sparkler(svc, {64.f, 32.f}, svc.styles.colors.ui_white, "bed");
	sparkler.set_position(position);
	bounding_box = shape::Shape({64.f, 32.f});
	bounding_box.set_position(position);
}

void Bed::update(automa::ServiceProvider& svc, world::Map& map, gui::Console& console, player::Player& player, flfx::Transition& transition) {
	fadeout.update();
	sparkler.update(svc);
	sparkler.set_position(bounding_box.position);
	if (player.collider.bounding_box.overlaps(bounding_box)) {
		flags.set(BedFlags::active);
		sparkler.activate();
		fadeout.start();
		if (player.controller.inspecting()) {
			console.set_source(svc.text.basic);
			console.load_and_launch("bed");
			flags.set(BedFlags::engaged);
		}
	} else {
		flags.reset(BedFlags::active);
		sparkler.deactivate();
	}
	if (svc.state_controller.actions.test(automa::Actions::sleep) && flags.test(BedFlags::engaged)) {
		svc.music.load("brown");
		svc.music.play_looped(10);
		transition.start();
		if (transition.is_done() && console.is_complete()) {
			player.health.heal(64.f);
			player.health.update();
			svc.soundboard.flags.item.set(audio::Item::heal);
			svc.music.load(svc.data.map_jsons.at(room).metadata["meta"]["music"].as_string());
			svc.music.play_looped(10);
			transition.end();
			svc.state_controller.actions.reset(automa::Actions::sleep);
			flags.reset(BedFlags::engaged);
		}
	}
}

void Bed::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (!fadeout.running()) { return; }
	sparkler.render(svc, win, cam);
}

} // namespace entity
