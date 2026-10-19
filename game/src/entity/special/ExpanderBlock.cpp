
#include <fornani/core/Debug.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/entity/SpecialBlock.hpp>
#include <fornani/entity/special/ExpanderBlock.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

ExpanderBlock::ExpanderBlock(automa::ServiceProvider& svc, dj::Json const& in) : SpecialBlock(svc, in) {
	unserialize(in);
	p_animatable.set_texture_rect(sf::IntRect{{}, constants::i_resolution_vec});
}

ExpanderBlock::ExpanderBlock(automa::ServiceProvider& svc, world::Map& map, dj::Json const& in) : SpecialBlock(svc, map, in) {
	unserialize(in);
	p_animatable.set_texture(svc.assets.get_texture("special_blocks"));
	p_animatable.set_texture_rect(sf::IntRect{{}, constants::i_resolution_vec});
	m_attributes.set(ExpanderBlockAttributes::unlit);
	if (!has_collider()) { return; }
	get_collider().set_position(get_world_position());
	get_collider().set_trait(shape::ColliderTrait::block);
	get_collider().set_attribute(shape::ColliderAttributes::fixed);
	get_collider().set_attribute(shape::ColliderAttributes::crusher);
}

ExpanderBlock::ExpanderBlock(automa::ServiceProvider& svc, sf::Vector2u dimensions, int id, std::string_view label, util::BitFlags<SpecialBlockAttributes> attributes) : SpecialBlock{svc, dimensions, id, label, attributes} {
	p_animatable.set_texture_rect(sf::IntRect{{}, constants::i_resolution_vec});
}

void ExpanderBlock::serialize(dj::Json& out) { SpecialBlock::serialize(out); }

void ExpanderBlock::unserialize(dj::Json const& in) { SpecialBlock::unserialize(in); }

void ExpanderBlock::update(automa::ServiceProvider& svc, world::Map& map, SceneContext& context, player::Player& player) {
	SpecialBlock::update(svc, map, context, player);
	if (!has_collider()) { return; }

	// expansion logic
	m_timer.update();
	if (m_timer.is_complete()) { m_timer.start(); }
	m_factor = 3.f;
	m_timer.get_normalized() < m_threshold ? m_flags.set(ExpanderBlockFlags::expanded) : m_flags.reset(ExpanderBlockFlags::expanded);
	auto const target_scale = m_flags.test(ExpanderBlockFlags::expanded) ? m_factor : 1.f;
	m_scale = std::lerp(m_scale, target_scale, 0.2f);
	auto const to_dim = constants::f_cell_vec * m_scale;
	get_collider().set_dimensions(to_dim);
	get_collider().sync_components();
	p_animatable.set_scale(constants::f_scale_vec * m_scale);
	get_collider().set_position(get_world_position() - constants::f_cell_vec * (m_scale - 1.f) / 2.f);

	get_collider().set_attribute(shape::ColliderAttributes::no_collision, ignore_updates());
	get_collider().set_flag(shape::ColliderFlags::intangible, ignore_updates());
	if (ignore_updates()) { return; }
	// player.get_collider().handle_collider_collision(get_collider());
}

void ExpanderBlock::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	// debug();
	if (ignore_updates()) { return; }
	highlighted ? drawbox.setFillColor(sf::Color{60, 255, 120, 180}) : drawbox.setFillColor(sf::Color{60, 255, 120, 80});
	SpecialBlock::render(win, cam, size);
	if (m_editor) { return; }
	p_animatable.set_texture_rect(sf::IntRect{{}, constants::i_resolution_vec});
	if (has_collider()) { p_animatable.set_position(get_collider().get_position() - cam); }
	win.draw(p_animatable);
}

void ExpanderBlock::render(sf::RenderTexture& tex, sf::Vector2f cam) {
	// debug();
	if (ignore_updates()) { return; }
	p_animatable.set_scale(constants::f_scale_vec);
	p_animatable.set_position(get_world_position());
	p_animatable.set_texture_rect(sf::IntRect{{}, constants::i_resolution_vec});
	tex.draw(p_animatable);
	++debug::draw_calls;
}

} // namespace fornani
