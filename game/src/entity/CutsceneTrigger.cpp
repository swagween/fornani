
#include <fornani/entities/player/Player.hpp>
#include <fornani/entity/CutsceneTrigger.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

CutsceneTrigger::CutsceneTrigger(automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "cutscene_triggers"), m_bounding_box{get_world_dimensions()} {
	unserialize(in);
	m_bounding_box.set_position(get_world_position());
	set_texture_rect(sf::IntRect{{}, constants::i_resolution_vec});
	repeatable = false;
}

CutsceneTrigger::CutsceneTrigger(automa::ServiceProvider& svc, sf::Vector2u dimensions, int id) : Entity(svc, "cutscene_triggers", id, dimensions) {
	set_texture_rect(sf::IntRect{{}, constants::i_resolution_vec});
	repeatable = false;
}

std::unique_ptr<Entity> CutsceneTrigger::clone() const { return std::make_unique<CutsceneTrigger>(*this); }

void CutsceneTrigger::serialize(dj::Json& out) {
	Entity::serialize(out);
	for (auto const& contingency : m_contingencies) { contingency.serialize(out["contingencies"]); }
}

void CutsceneTrigger::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	for (auto const& contingency : in["contingencies"].as_array()) { m_contingencies.push_back(QuestContingency{contingency}); }
}

void CutsceneTrigger::expose() {
	Entity::expose();
	ImGui::Text("Quest Contingencies");
	static char tag_buffer[256] = "";
	static int requirement{};
	static bool strict{};
	ImGui::InputTextWithHint("Tag", "Quest Tag", tag_buffer, IM_ARRAYSIZE(tag_buffer));
	ImGui::InputInt("Requirement", &requirement);
	ImGui::Checkbox("Strict?", &strict);
	if (ImGui::Button("Add Contingency")) { m_contingencies.push_back(QuestContingency{tag_buffer, requirement, strict}); }
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::Text("Current List:");
	for (auto [i, ct] : std::views::enumerate(m_contingencies)) {
		ImGui::PushID(i);
		if (ImGui::SmallButton("x")) { ct.delete_me = true; }
		ImGui::SameLine();
		ImGui::Text("[%s, %i]", ct.tag.c_str(), ct.requirement);
		ImGui::PopID();
	}
	std::erase_if(m_contingencies, [](auto const& ct) { return ct.delete_me; });
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

void CutsceneTrigger::update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, [[maybe_unused]] player::Player& player) {
	if (!svc.quest_table.are_contingencies_met(m_contingencies)) { return; }
	if (player.collider.bounding_box.overlaps(m_bounding_box) && !is_pushed()) { m_flags.set(CutsceneTriggerFlags::activated); }
	if (is_activated()) {
		map.cutscene_catalog.push_cutscene(svc, map, get_id());
		m_flags.reset(CutsceneTriggerFlags::activated);
		m_flags.set(CutsceneTriggerFlags::pushed);
	}
}

} // namespace fornani
