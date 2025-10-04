#include "fornani/entities/world/Bed.hpp"
#include <fornani/gui/console/Console.hpp>
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::entity {

constexpr auto bed_dimensions_v = sf::Vector2f{64.f, 32.f};

Bed::Bed(automa::ServiceProvider& svc, sf::Vector2f position, int style, bool flipped) : m_sprite{svc.assets.get_texture("beds")}, sparkler{svc, bed_dimensions_v, colors::ui_white, "bed"} {
	sparkler.set_position(position);
	bounding_box = shape::Shape(bed_dimensions_v);
	bounding_box.set_position(position);
	m_sprite.setTextureRect(sf::IntRect{{0, 16 * style}, sf::Vector2i{bed_dimensions_v / constants::f_scale_factor}});
	flipped ? m_sprite.setScale(constants::f_scale_vec) : m_sprite.setScale(constants::f_inverse_scale_vec);
	m_sprite.setOrigin(m_sprite.getLocalBounds().getCenter());
}

constexpr auto sleep_timer_v = 100;
constexpr auto rest_time_v = 200;

void Bed::update(automa::ServiceProvider& svc, world::Map& map, std::optional<std::unique_ptr<gui::Console>>& console, player::Player& player, graphics::Transition& transition) {
	fadeout.update();
	sparkler.update(svc);
	sparkler.set_position(bounding_box.get_position());
	if (player.is_busy() || player.is_in_custom_sleep_event()) { return; }

	if (player.collider.bounding_box.overlaps(bounding_box)) {
		flags.set(BedFlags::active);
		sparkler.activate();
		fadeout.start();
		if (!flags.test(BedFlags::engaged) && !flags.test(BedFlags::slept_in) && player.controller.inspecting() && !player.is_busy()) { player.flags.state.set(player::State::sleep); }
		if (player.is_in_animation(player::AnimState::sleep) && !console && player.animation.animation.get_elapsed_ticks() >= sleep_timer_v && !flags.test(BedFlags::slept_in)) {
			if (!flags.test(BedFlags::engaged)) { transition.start(); }
			flags.set(BedFlags::engaged);
		}
	} else {
		flags.reset(BedFlags::active);
		sparkler.deactivate();
	}
	if (flags.test(BedFlags::engaged)) {
		svc.music_player.pause();
		svc.data.respawn_all(); // respawn enemies
		if (transition.has_waited(rest_time_v) && !console) {
			flags.set(BedFlags::slept_in);
			console = std::make_unique<gui::Console>(svc, svc.text.basic, "bed", gui::OutputType::gradual);
			player.health.refill();
			svc.soundboard.flags.item.set(audio::Item::heal);
			svc.music_player.play_looped();
			transition.end();
			flags.reset(BedFlags::engaged);
		}
	}
	if (!console && flags.test(BedFlags::slept_in)) {
		player.flags.state.set(player::State::wake_up);
		flags.reset(BedFlags::slept_in);
	}
}

void Bed::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	m_sprite.setPosition(bounding_box.get_center() - cam);
	win.draw(m_sprite);
	if (!fadeout.running()) { return; }
	sparkler.render(win, cam);
}

} // namespace fornani::entity
