#include "fornani/entities/world/Bed.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/gui/Console.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::entity {

Bed::Bed(automa::ServiceProvider& svc, sf::Vector2<float> position, int room) : room(room) {
	sparkler = vfx::Sparkler(svc, {64.f, 32.f}, svc.styles.colors.ui_white, "bed");
	sparkler.set_position(position);
	bounding_box = shape::Shape({64.f, 32.f});
	bounding_box.set_position(position);
}

void Bed::update(automa::ServiceProvider& svc, world::Map& map, std::optional<std::unique_ptr<gui::Console>>& console, player::Player& player, flfx::Transition& transition) {
	fadeout.update();
	sparkler.update(svc);
	sparkler.set_position(bounding_box.get_position());
	if (player.collider.bounding_box.overlaps(bounding_box)) {
		flags.set(BedFlags::active);
		sparkler.activate();
		fadeout.start();
		if (player.controller.inspecting()) {
			console = std::make_unique<gui::Console>(svc, svc.text.basic, "bed", gui::OutputType::gradual);
			flags.set(BedFlags::engaged);
		}
	} else {
		flags.reset(BedFlags::active);
		sparkler.deactivate();
	}
	if (svc.state_controller.actions.test(automa::Actions::sleep) && flags.test(BedFlags::engaged)) {
		svc.music.load(svc.finder, "brown");
		svc.music.play_looped(10);
		transition.start();
		svc.data.respawn_all();
		if (transition.is_done() && !console) {
			player.health.heal(64.f);
			player.health.update();
			svc.soundboard.flags.item.set(audio::Item::heal);
			svc.music.load(svc.finder, svc.data.map_jsons.at(room).metadata["meta"]["music"].as_string());
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

} // namespace fornani::entity
