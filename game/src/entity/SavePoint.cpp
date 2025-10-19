
#include <fornani/entities/player/Player.hpp>
#include <fornani/entity/SavePoint.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani {

SavePoint::SavePoint(fornani::automa::ServiceProvider& svc, dj::Json const& in)
	: Entity(svc, in, "save_point"), m_anim_params{0, 12, 24, -1}, bounding_box{constants::f_cell_vec}, proximity_box{constants::f_cell_vec * 8.f}, sparkler{svc, constants::f_cell_vec, colors::green, "save_point"} {
	unserialize(in);
	set_texture_rect(sf::IntRect{{}, fornani::constants::i_resolution_vec});
	unique = true;

	set_parameters(m_anim_params);
	sparkler.set_position(get_world_position());
}

SavePoint::SavePoint(fornani::automa::ServiceProvider& svc, int id) : Entity(svc, "save_point", id, {1, 1}), sparkler{svc, constants::f_cell_vec, colors::green, "save_point"} {
	unique = true;
	set_texture_rect(sf::IntRect{{}, fornani::constants::i_resolution_vec});
}

std::unique_ptr<Entity> SavePoint::clone() const { return std::make_unique<SavePoint>(*this); }

void SavePoint::serialize(dj::Json& out) { Entity::serialize(out); }

void SavePoint::unserialize(dj::Json const& in) { Entity::unserialize(in); }

void SavePoint::expose() { Entity::expose(); }

void SavePoint::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{250, 10, 250, 100}) : drawbox.setFillColor(sf::Color{250, 10, 250, 60});
	Entity::render(win, cam, size);
	auto offset = sf::Vector2f{-constants::f_cell_vec.x * 0.5f, -get_f_dimensions().y};
	Animatable::set_position(get_world_position() - cam + offset);
	win.draw(*this);
	sparkler.render(win, cam);
}

void SavePoint::update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, [[maybe_unused]] player::Player& player) {
	Entity::update(svc, map, console, player);
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
					svc.state_controller.save_point_id = get_id();
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

void SavePoint::save(automa::ServiceProvider& svc, player::Player& player) {
	svc.data.save_progress(player, get_id());
	can_activate = false;
}

} // namespace fornani
