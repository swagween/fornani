
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/entity/SavePoint.hpp>
#include <fornani/graphics/Renderer.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani {

SavePoint::SavePoint(fornani::automa::ServiceProvider& svc, dj::Json const& in)
	: Entity(svc, in, "save_point"), m_anim_params{0, 12, 24, -1}, bounding_box{constants::f_cell_vec}, proximity_box{constants::f_cell_vec * 8.f}, sparkler{svc, constants::f_cell_vec, colors::green, "save_point"} {
	unserialize(in);
	p_animatable.set_texture_rect(sf::IntRect{{}, fornani::constants::i_resolution_vec});
	unique = true;

	p_animatable.set_parameters(m_anim_params);
	sparkler.set_position(get_world_position());
}

SavePoint::SavePoint(fornani::automa::ServiceProvider& svc, int id) : Entity(svc, "save_point", id, {1, 1}), sparkler{svc, constants::f_cell_vec, colors::green, "save_point"} {
	unique = true;
	p_animatable.set_texture_rect(sf::IntRect{{}, fornani::constants::i_resolution_vec});
}

std::unique_ptr<Entity> SavePoint::clone() const { return std::make_unique<SavePoint>(*this); }

void SavePoint::serialize(dj::Json& out) { Entity::serialize(out); }

void SavePoint::unserialize(dj::Json const& in) { Entity::unserialize(in); }

void SavePoint::expose() { Entity::expose(); }

void SavePoint::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{250, 10, 250, 100}) : drawbox.setFillColor(sf::Color{250, 10, 250, 60});
	Entity::render(win, cam, size);
	auto offset = sf::Vector2f{-constants::f_cell_vec.x * 0.5f, -p_animatable.get_f_dimensions().y};
	p_animatable.set_position(get_world_position() - cam + offset);
	win.draw(p_animatable);
}

void SavePoint::submit(Renderer& renderer) {
	auto offset = sf::Vector2f{-constants::f_cell_vec.x * 0.5f, -p_animatable.get_f_dimensions().y};
	auto const pos = get_world_position() + offset;
	auto const& frame = p_animatable.get_sprite().getTextureRect();

	sf::FloatRect dest{pos, sf::Vector2f{frame.size}};
	renderer.submit(p_animatable.get_sprite().getTexture(), dest, frame, RenderLayer::background_entities);
	sparkler.submit(renderer);
}

void SavePoint::update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] SceneContext& context, [[maybe_unused]] player::Player& player) {
	Entity::update(svc, map, context, player);
	sparkler.update(svc);
	intensity < 2 ? sparkler.set_color(colors::periwinkle) : sparkler.set_color(colors::ui_white);

	sf::Vector2f proximity_offset = (proximity_box.get_dimensions() - bounding_box.get_dimensions()) * 0.5f;
	sparkler.set_position(get_world_position());
	bounding_box.set_position(get_world_position());
	proximity_box.set_position(get_world_position() - proximity_offset);
	activated = false;

	svc.soundboard.repeat_sound("save_point", 1U, bounding_box.get_center());
	if (player.get_collider().bounding_box.overlaps(proximity_box)) {
		if (player.get_collider().bounding_box.overlaps(bounding_box)) {
			intensity = 3;
			if (p_animatable.animation.keyframe_over()) { p_animatable.animation.params.framerate = 4; }
			if (player.controller.inspecting()) {
				if (can_activate) {
					activated = true;
					save(svc, player);
					svc.state_controller.save_point_id = get_id();
					svc.soundboard.flags.world.set(audio::World::save);
					context.console = std::make_unique<gui::Console>(svc, svc.text.basic, "save", gui::OutputType::gradual);
				}
			}
		} else {
			intensity = 2;
			if (p_animatable.animation.keyframe_over()) { p_animatable.animation.params.framerate = 8; }
			can_activate = true;
		}
	} else {
		intensity = 1;
		if (p_animatable.animation.keyframe_over()) { p_animatable.animation.params.framerate = 12; }
	}
	p_animatable.set_channel(intensity);
}

void SavePoint::save(automa::ServiceProvider& svc, player::Player& player) {
	svc.data.save_progress(player, get_id());
	can_activate = false;
}

} // namespace fornani
