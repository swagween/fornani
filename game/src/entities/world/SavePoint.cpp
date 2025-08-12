
#include "fornani/entities/world/SavePoint.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/gui/Console.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::entity {

SavePoint::SavePoint(automa::ServiceProvider& svc, int id, Vecu32 position)
	: IWorldPositionable(position), Animatable(svc, "save_point", {32, 32}), m_id{id}, m_anim_params{0, 12, 24, -1}, bounding_box{constants::f_cell_vec}, proximity_box{constants::f_cell_vec * 8.f} {

	set_parameters(m_anim_params);

	sparkler = vfx::Sparkler(svc, constants::f_cell_vec, colors::green, "save_point");
	sparkler.set_position(get_world_position());
}

void SavePoint::update(automa::ServiceProvider& svc, player::Player& player, std::optional<std::unique_ptr<gui::Console>>& console) {

	Animatable::tick();
	sparkler.update(svc);
	intensity < 2 ? sparkler.set_color(colors::periwinkle) : sparkler.set_color(colors::ui_white);

	sf::Vector2f proximity_offset = (proximity_box.get_dimensions() - bounding_box.get_dimensions()) * 0.5f;
	sparkler.set_position(get_world_position());
	bounding_box.set_position(get_world_position());
	proximity_box.set_position(get_world_position() - proximity_offset);
	activated = false;

	if (player.collider.bounding_box.overlaps(proximity_box)) {
		if (player.collider.bounding_box.overlaps(bounding_box)) {
			intensity = 3;
			if (animation.keyframe_over()) { animation.params.framerate = 4; }
			if (player.controller.inspecting()) {
				if (can_activate) {
					activated = true;
					save(svc, player);
					svc.state_controller.save_point_id = m_id;
					svc.soundboard.flags.world.set(audio::World::save);
					console = std::make_unique<gui::Console>(svc, svc.text.basic, "save", gui::OutputType::gradual);
				}
			}
		} else {
			intensity = 2;
			if (animation.keyframe_over()) { animation.params.framerate = 8; }
			can_activate = true;
		}
	} else {
		intensity = 1;
		if (animation.keyframe_over()) { animation.params.framerate = 12; }
	}
	set_channel(intensity);
}

void SavePoint::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f campos) {
	auto offset = sf::Vector2f{-constants::f_cell_vec.x * 0.5f, -get_f_dimensions().y};
	Animatable::set_position(get_world_position() - campos + offset);
	win.draw(*this);
	sparkler.render(svc, win, campos);
	if (svc.greyblock_mode()) {
		sf::RectangleShape box{};
		box.setFillColor(sf::Color::Transparent);
		box.setPosition(proximity_box.get_position() - campos);
		box.setSize(proximity_box.get_dimensions());
		box.setOutlineColor(sf::Color::Blue);
		box.setOutlineThickness(-2.f);
		win.draw(box);
		box.setPosition(bounding_box.get_position() - campos);
		box.setSize(bounding_box.get_dimensions());
		box.setOutlineColor(sf::Color::Green);
		box.setOutlineThickness(-2.f);
		win.draw(box);
	}
}

void SavePoint::save(automa::ServiceProvider& svc, player::Player& player) {
	svc.data.save_progress(player, m_id);
	can_activate = false;
}

} // namespace fornani::entity
