
#include <imgui.h>
#include <fornani/entity/Entity.hpp>
#include <fornani/graphics/Colors.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Constants.hpp>

namespace fornani {

Entity::Entity(automa::ServiceProvider& svc, dj::Json const& in, std::string_view label) : Drawable(svc, label), m_label{label} {
	unserialize(in);
	m_editor = svc.is_editor();
}

Entity::Entity(automa::ServiceProvider& svc, std::string_view label, int to_id, sf::Vector2<std::uint32_t> dim) : Drawable(svc, label), m_id{to_id}, m_label{label}, dimensions{dim} {}

std::unique_ptr<Entity> Entity::clone() const { return std::unique_ptr<Entity>(); }

void Entity::serialize(dj::Json& out) {
	out["id"] = m_id;
	out["position"][0] = position.x;
	out["position"][1] = position.y;
	out["dimensions"][0] = dimensions.x;
	out["dimensions"][1] = dimensions.y;
}

void Entity::unserialize(dj::Json const& in) {
	m_id = in["id"].as<int>();
	NANI_LOG_INFO(m_logger, "Unserializing entity with id {}", m_id);
	position.x = in["position"][0].as<std::uint32_t>();
	position.y = in["position"][1].as<std::uint32_t>();
	dimensions.x = in["dimensions"][0].as<std::uint32_t>();
	dimensions.y = in["dimensions"][1].as<std::uint32_t>();
}

void Entity::expose() {
	ImGui::Text("Category: %s", m_label.c_str());
	ImGui::Separator();
	ImGui::InputInt("Entity ID", &m_id);
	ImGui::Text("Position: (%i", position.x);
	ImGui::SameLine();
	ImGui::Text(", %i)", position.y);
	ImGui::Separator();
}

void Entity::set_position(sf::Vector2u to_position) { position = to_position; }

void Entity::update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, [[maybe_unused]] player::Player& player) {}

void Entity::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	drawbox.setOutlineColor(colors::blue);
	drawbox.setOutlineThickness(-2.f);
	if (highlighted) {
		drawbox.setOutlineColor(colors::ui_white);
		drawbox.setOutlineThickness(2.f);
	}
	drawbox.setSize(f_dimensions() * size);
	drawbox.setPosition(sf::Vector2f{position} * size + cam);
	Drawable::set_scale(constants::f_scale_vec * size / constants::f_cell_size);
	Drawable::set_position(sf::Vector2f{position} * size + cam);
	if (m_textured) { win.draw(*this); }
	if (m_editor) { win.draw(drawbox); }
}

} // namespace fornani
