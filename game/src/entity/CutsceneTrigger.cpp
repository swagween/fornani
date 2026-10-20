
#include <fornani/entities/player/Player.hpp>
#include <fornani/entity/CutsceneTrigger.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/AirTravel.hpp>
#include <fornani/story/cutscene/BattleArena.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

CutsceneTrigger::CutsceneTrigger(automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "cutscene_triggers"), m_bounding_box{get_world_dimensions()}, m_hover_trigger{100} {
	unserialize(in);
	m_bounding_box.set_position(get_world_position());
	p_animatable.set_texture_rect(sf::IntRect{{}, constants::i_resolution_vec});
	repeatable = false;
}

CutsceneTrigger::CutsceneTrigger(automa::ServiceProvider& svc, sf::Vector2u dimensions, int id, util::BitFlags<CutsceneTriggerAttributes> attributes)
	: Entity(svc, "cutscene_triggers", id, dimensions), m_attributes{attributes}, m_hover_trigger{100} {
	p_animatable.set_texture_rect(sf::IntRect{{}, constants::i_resolution_vec});
	repeatable = false;
}

std::unique_ptr<Entity> CutsceneTrigger::clone() const { return std::make_unique<CutsceneTrigger>(*this); }

void CutsceneTrigger::serialize(dj::Json& out) {
	Entity::serialize(out);
	if (m_attributes.test(CutsceneTriggerAttributes::callbox)) { out["attributes"]["callbox"] = true; }
	if (m_attributes.test(CutsceneTriggerAttributes::persistent)) { out["attributes"]["persistent"] = true; }
}

void CutsceneTrigger::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	if (in["attributes"]["callbox"].as_bool()) { m_attributes.set(CutsceneTriggerAttributes::callbox); }
	if (in["attributes"]["persistent"].as_bool()) { m_attributes.set(CutsceneTriggerAttributes::persistent); }
	if (get_id() == arena_id_v) { m_attributes.set(CutsceneTriggerAttributes::arena); }
	if (get_id() == air_travel_id_v) { m_attributes.set(CutsceneTriggerAttributes::persistent); }
}

void CutsceneTrigger::expose() {
	Entity::expose();
	static bool c{};
	static bool p{};
	ImGui::Checkbox("Callbox?", &c);
	ImGui::Checkbox("Persistent?", &p);
	c ? m_attributes.set(CutsceneTriggerAttributes::callbox) : m_attributes.reset(CutsceneTriggerAttributes::callbox);

	p ? m_attributes.set(CutsceneTriggerAttributes::persistent) : m_attributes.reset(CutsceneTriggerAttributes::persistent);
}

void CutsceneTrigger::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{60, 255, 120, 180}) : drawbox.setFillColor(sf::Color{60, 255, 120, 80});
	Entity::render(win, cam, size);
	if (!m_editor) {
		auto color = is_activated() ? sf::Color(sf::Color{60, 255, 120, 180}) : sf::Color(sf::Color{255, 60, 120, 80});
		drawbox.setSize(get_world_dimensions() * size);
		drawbox.setPosition(get_world_dimensions() * size + cam);
		win.draw(drawbox);
		m_bounding_box.render(win, cam, color);
	}
}

void CutsceneTrigger::update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] SceneContext& context, [[maybe_unused]] player::Player& player) {
	if (p_contingencies) {
		if (!svc.quest_table.are_contingencies_met(*p_contingencies)) { return; }
	}
	m_hover_trigger.update();
	if (player.get_collider().bounding_box.overlaps(m_bounding_box) && (!is_pushed() || m_attributes.test(CutsceneTriggerAttributes::persistent))) {
		if (m_attributes.test(CutsceneTriggerAttributes::callbox)) {
			if (player.controller.inspecting()) { m_flags.set(CutsceneTriggerFlags::activated); }
			if (!m_flags.test(CutsceneTriggerFlags::hovered)) {
				if (!m_hover_trigger.running() && !player.has_flag_set(player::PlayerFlags::cutscene)) { map.spawn_effect(svc, "question_mark", get_global_center() + sf::Vector2f{0.f, -32.f}); }
				m_flags.set(CutsceneTriggerFlags::hovered);
				m_hover_trigger.start();
			}
		} else {
			m_flags.set(CutsceneTriggerFlags::activated);
		}
	} else {
		m_flags.reset(CutsceneTriggerFlags::hovered);
	}
	if (is_activated()) {
		if (get_id() != 0) {
			auto const id = m_attributes.test(CutsceneTriggerAttributes::arena) ? arena_id_v : get_id();
			svc.events.launch_cutscene_event.dispatch(svc, id);
		}
		m_flags.reset(CutsceneTriggerFlags::activated);
		m_flags.set(CutsceneTriggerFlags::pushed);
	}
}

} // namespace fornani
