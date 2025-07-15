
#include "editor/canvas/Entity.hpp"
#include <imgui.h>
#include <fornani/graphics/Colors.hpp>
#include <fornani/utils/Constants.hpp>

namespace pi {

Entity::Entity(fornani::automa::ServiceProvider& svc, dj::Json const& in, std::string_view label) : Drawable(svc, label), m_label{label} { unserialize(in); }

Entity::Entity(fornani::automa::ServiceProvider& svc, std::string_view label, int to_id, sf::Vector2<std::uint32_t> dim) : Drawable(svc, label), id{to_id}, m_label{label}, dimensions{dim} {}

std::unique_ptr<Entity> Entity::clone() const { return std::unique_ptr<Entity>(); }

void Entity::serialize(dj::Json& out) {
	out["id"] = id;
	out["position"][0] = position.x;
	out["position"][1] = position.y;
	out["dimensions"][0] = dimensions.x;
	out["dimensions"][1] = dimensions.y;
}

void Entity::unserialize(dj::Json const& in) {
	id = in["id"].as<int>();
	NANI_LOG_INFO(m_logger, "Unserializing entity with id {}", id);
	position.x = in["position"][0].as<std::uint32_t>();
	position.y = in["position"][1].as<std::uint32_t>();
	dimensions.x = in["dimensions"][0].as<std::uint32_t>();
	dimensions.y = in["dimensions"][1].as<std::uint32_t>();
}

void Entity::expose() {
	ImGui::Text("Category: %s", m_label.c_str());
	ImGui::Separator();
	ImGui::InputInt("Entity ID", &id);
	ImGui::Text("Position: (%i", position.x);
	ImGui::SameLine();
	ImGui::Text(", %i)", position.y);
	ImGui::Separator();
}

void Entity::set_position(sf::Vector2u to_position) { position = to_position; }

void Entity::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	drawbox.setOutlineColor(fornani::colors::blue);
	drawbox.setOutlineThickness(-2.f);
	if (highlighted) {
		drawbox.setOutlineColor(fornani::colors::ui_white);
		drawbox.setOutlineThickness(2.f);
	}
	drawbox.setSize(f_dimensions() * size);
	drawbox.setPosition(sf::Vector2f{position} * size + cam);
	Drawable::set_scale(fornani::constants::f_scale_vec * size / fornani::constants::f_cell_size);
	Drawable::set_position(sf::Vector2f{position} * size + cam);
	if (m_textured) { win.draw(*this); }
	win.draw(drawbox);
}

} // namespace pi
