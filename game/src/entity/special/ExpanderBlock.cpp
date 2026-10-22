
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
	auto const tag = m_specs.factor < 4.f ? "expander_blocks" : "large_expander_blocks";
	p_animatable.set_texture(svc.assets.get_texture(tag));
	auto const dim = constants::i_resolution_vec * static_cast<int>(m_specs.factor) + sf::Vector2i{4, 4};
	p_animatable.set_texture_rect(sf::IntRect{{}, dim});
	// m_attributes.set(ExpanderBlockAttributes::unlit);
	if (!has_collider()) { return; }
	get_collider().set_position(get_world_position());
	get_collider().set_trait(shape::ColliderTrait::block);
	get_collider().set_attribute(shape::ColliderAttributes::fixed);
	get_collider().set_attribute(shape::ColliderAttributes::crusher);
	p_animatable.center();
	p_animatable.set_dimensions(dim);
	p_animatable.push_animation("open", {0, 4, 24, -1, true});
	p_animatable.push_animation("close", {4, 5, 24, -1, true});

	if (m_specs.delay > constants::tiny_value) { m_timer.start(static_cast<int>(m_specs.time * m_specs.delay)); }
}

ExpanderBlock::ExpanderBlock(automa::ServiceProvider& svc, sf::Vector2u dimensions, int id, std::string_view label, ExpanderBlockSpecifications specs, util::BitFlags<SpecialBlockAttributes> attributes)
	: SpecialBlock{svc, dimensions, id, label, attributes}, m_specs{specs}, m_timer{specs.time} {
	p_animatable.set_texture_rect(sf::IntRect{{}, constants::i_resolution_vec});
}

void ExpanderBlock::expose() {
	ImGui::InputInt("Time", &m_specs.time);
	ImGui::InputFloat("Threshold", &m_specs.threshold);
	ImGui::InputFloat("Factor", &m_specs.factor);
	ImGui::InputFloat("Delay", &m_specs.delay);
}

void ExpanderBlock::serialize(dj::Json& out) {
	SpecialBlock::serialize(out);
	out["time"] = m_specs.time;
	out["threshold"] = m_specs.threshold;
	out["factor"] = m_specs.factor;
	out["delay"] = m_specs.delay;
}

void ExpanderBlock::unserialize(dj::Json const& in) {
	SpecialBlock::unserialize(in);
	m_specs.time = in["time"].as<int>();
	m_specs.threshold = in["threshold"].as<float>();
	m_specs.factor = in["factor"].as<float>();
	m_specs.delay = in["delay"].as<float>();
	m_timer = util::Cooldown{m_specs.time};
}

void ExpanderBlock::update(automa::ServiceProvider& svc, world::Map& map, SceneContext& context, player::Player& player) {
	SpecialBlock::update(svc, map, context, player);
	if (!has_collider()) { return; }

	// expansion logic
	auto const& data = map.get_biome_properties()["expander_blocks"];
	m_timer.update();
	if (m_timer.is_complete()) { m_timer.start(); }
	if (m_timer.get_normalized() < m_specs.threshold) {
		if (!m_flags.test(ExpanderBlockFlags::expanded)) {
			p_animatable.set_animation("open");
			auto const expand_sound = random::element(data["sounds"]["expand"].as_array()).as_string_view();
			svc.soundboard.play_sound(expand_sound, get_collider().get_center());
		}
		m_flags.set(ExpanderBlockFlags::expanded);
	} else {
		if (m_flags.test(ExpanderBlockFlags::expanded)) {
			p_animatable.set_animation("close");
			auto const contract_sound = random::element(data["sounds"]["contract"].as_array()).as_string_view();
			svc.soundboard.play_sound(contract_sound, get_collider().get_center());
		}
		m_flags.reset(ExpanderBlockFlags::expanded);
	}
	auto const target_scale = m_timer.get_normalized() < m_specs.threshold - 0.1f ? m_specs.factor : 1.f;
	m_scale = std::lerp(m_scale, target_scale, 0.2f);
	auto const to_dim = constants::f_cell_vec * m_scale;
	get_collider().set_dimensions(to_dim);
	get_collider().sync_components();
	// p_animatable.set_scale(constants::f_scale_vec * m_scale);
	get_collider().set_position(get_world_position() - constants::f_cell_vec * (m_scale - 1.f) / 2.f);

	get_collider().set_attribute(shape::ColliderAttributes::no_collision, ignore_updates());
	get_collider().set_flag(shape::ColliderFlags::intangible, ignore_updates());
}

void ExpanderBlock::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	// debug();
	if (ignore_updates()) { return; }
	highlighted ? drawbox.setFillColor(sf::Color{60, 255, 120, 180}) : drawbox.setFillColor(sf::Color{60, 255, 120, 80});
	SpecialBlock::render(win, cam, size);
	if (m_editor) { return; }
	if (has_collider()) { p_animatable.set_position(get_collider().get_center() - cam); }
	win.draw(p_animatable);
	// get_collider().render(win, cam);
}

void ExpanderBlock::render(sf::RenderTexture& tex, sf::Vector2f cam) {
	// debug();
	if (ignore_updates()) { return; }
	p_animatable.set_scale(constants::f_scale_vec);
	p_animatable.set_position(get_collider().get_center());
	tex.draw(p_animatable);
	++debug::draw_calls;
}

} // namespace fornani
