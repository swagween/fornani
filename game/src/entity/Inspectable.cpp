
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/entities/world/Inspectable.hpp>
#include <fornani/entity/Inspectable.hpp>
#include <fornani/events/ConsoleEvent.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

Inspectable::Inspectable(automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "inspectables") {
	unserialize(in);
	if (in["activate_on_contact"].as_bool()) { attributes.set(InspectableAttributes::activate_on_contact); }
	if (in["instant"].as_bool()) { attributes.set(InspectableAttributes::instant); }
	m_key = in["key"].as_string();
	alternates = in["alternates"].as<int>();
	m_textured = false;
	svc.events.destroy_inspectable_event.attach_to(slot, &Inspectable::destroy_me, this);
}

Inspectable::Inspectable(automa::ServiceProvider& svc, std::vector<std::vector<gui::BasicMessage>> suite, std::vector<std::vector<gui::BasicMessage>> responses, bool activate_on_contact, std::string key, int alternates, bool instant)
	: Entity(svc, "inspectables", 0, {1, 1}), m_activate_on_contact{activate_on_contact}, m_key{key}, m_alternates{alternates}, m_instant{instant}, m_suites{suite}, m_responses{responses} {
	m_textured = false;
}

std::unique_ptr<Entity> Inspectable::clone() const { return std::make_unique<Inspectable>(*this); }

void Inspectable::serialize(dj::Json& out) {
	Entity::serialize(out);
	auto const& wipe = dj::Json::empty_array();
	out["activate_on_contact"] = m_activate_on_contact;
	out["instant"] = m_instant;
	out["key"] = m_key;
	out["alternates"] = m_alternates;
	out["series"] = dj::Json::empty_array();
	for (auto i{0}; i < 1; ++i) {
		auto next = dj::Json{};
		for (auto j = 0; j < 2; ++j) {
			auto& from_set = j == 0 ? m_suites : m_responses;
			auto tag = j == 0 ? "suite" : "responses";
			for (auto& suite : from_set) {
				auto out_set = dj::Json{};
				for (auto const& message : suite) {
					auto msg = dj::Json{};
					if (message.contingencies) {
						for (auto const& cont : message.contingencies.value()) { cont.serialize(msg["contingencies"]); }
					}
					if (message.codes) {
						for (auto const& code : message.codes.value()) { code.serialize(msg["codes"]); }
					}
					msg["message"] = message.data;
					out_set.push_back(msg);
				}
				next[tag].push_back(out_set);
			}
		}
		out["series"].push_back(next);
		NANI_LOG_DEBUG(m_logger, "Serialized a series entry.");
	}
}

void Inspectable::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	auto const& wipe = dj::Json::empty_array();
	m_activate_on_contact = in["activate_on_contact"].as_bool();
	m_instant = in["instant"].as_bool();
	m_key = in["key"].as_string();
	m_alternates = in["alternates"].as<int>();
	m_suites.clear();
	m_responses.clear();
	for (auto const& entry : in["series"].as_array()) {
		for (auto j = 0; j < 2; ++j) {
			auto& to_set = j == 0 ? m_suites : m_responses;
			auto tag = j == 0 ? "suite" : "responses";
			for (auto const& in_suite : entry[tag].as_array()) {
				auto s = std::vector<gui::BasicMessage>{};
				for (auto const& message : in_suite.as_array()) {
					auto codes = std::vector<gui::MessageCode>{};
					auto contingencies = std::vector<QuestContingency>{};
					for (auto const& c : message["codes"].as_array()) {
						auto this_code = gui::MessageCode{c};
						codes.push_back(this_code);
					}
					for (auto const& c : message["contingencies"].as_array()) {
						auto this_cont = QuestContingency{c};
						contingencies.push_back(this_cont);
					}
					s.push_back(gui::BasicMessage{message["message"].as_string(), codes, contingencies});
				}
				to_set.push_back(s);
			}
		}
	}
}

void Inspectable::expose() {
	Entity::expose();
	ImGui::Checkbox("Activate on Contact", &m_activate_on_contact);
	ImGui::Checkbox("Instant", &m_instant);
	ImGui::InputInt("Alternates", &m_alternates);
}

void Inspectable::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	if (m_editor) {
		highlighted ? drawbox.setFillColor(sf::Color{255, 100, 60, 180}) : drawbox.setFillColor(sf::Color{255, 60, 60, 80});
		Entity::render(win, cam, size);
	}
}

void Inspectable::update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] SceneContext& context, [[maybe_unused]] player::Player& player) {
	auto const& set = map.get_json_data(svc)["entities"]["inspectables"][m_index];
	bounding_box.set_position(get_world_position());
	flags.reset(InspectableFlags::activated);
	if (m_indicator_cooldown.is_almost_complete()) { flags.reset(InspectableFlags::hovered); }
	if (svc.data.inspectable_is_destroyed(p_stable_id.get())) { flags.set(InspectableFlags::destroy); }
	if (destroyed()) { return; }
	if (spawn_denied()) { return; }

	// check for quest-based alternates
	/*auto quest_status = svc.quest.get_progression(quest::QuestType::inspectable, native_id);
	if (quest_status > 0) { current_alt = quest_status; }*/

	if (bounding_box.overlaps(player.get_collider().hurtbox)) {
		if (!flags.test(InspectableFlags::hovered)) { flags.set(InspectableFlags::hovered_trigger); }
		flags.set(InspectableFlags::hovered);
		if (attributes.test(InspectableAttributes::activate_on_contact) && flags.test(InspectableFlags::can_engage) && player.get_collider().grounded()) {
			flags.set(InspectableFlags::activated);
			flags.reset(InspectableFlags::can_engage);
		}
		if (player.controller.inspecting()) { flags.set(InspectableFlags::activated); }
	} else {
		m_indicator_cooldown.update();
		if (!m_indicator_cooldown.running()) { m_indicator_cooldown.start(); }
		flags.set(InspectableFlags::can_engage);
	}
	if (flags.test(InspectableFlags::activated) && !player.is_busy()) {
		player.set_busy(true);
		auto output_type = attributes.test(InspectableAttributes::instant) ? gui::OutputType::instant : gui::OutputType::gradual;
		context.console = std::make_unique<gui::Console>(p_stable_id, svc, set["series"][current_alt], output_type);
	}

	if (flags.test(InspectableFlags::hovered) && flags.consume(InspectableFlags::hovered_trigger) && !attributes.test(InspectableAttributes::activate_on_contact)) {
		map.spawn_effect(svc, "question_mark", get_global_center() + sf::Vector2f{0.f, -32.f});
	}
	if (context.console) {
		if (context.console.value()->get_key() == m_key) { flags.set(InspectableFlags::engaged); }
	}
	if (!context.console) {
		flags.reset(InspectableFlags::engaged);
		player.set_busy(false);
	}
}

void Inspectable::destroy_me(automa::ServiceProvider& svc) {
	svc.data.destroy_inspectable(p_stable_id.get());
	flags.set(InspectableFlags::destroy);
}

} // namespace fornani
