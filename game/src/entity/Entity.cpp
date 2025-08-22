
#include <imgui.h>
#include <fornani/entity/Entity.hpp>
#include <fornani/graphics/Colors.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Constants.hpp>

namespace fornani {

Entity::Entity(automa::ServiceProvider& svc, dj::Json const& in, std::string_view label)
	: Animatable(svc, label), m_label{label}, IWorldPositionable({in["position"][0].as<std::uint32_t>(), in["position"][1].as<std::uint32_t>()}, {in["dimensions"][0].as<std::uint32_t>(), in["dimensions"][1].as<std::uint32_t>()}) {
	unserialize(in);
	m_editor = svc.is_editor();
}

Entity::Entity(automa::ServiceProvider& svc, std::string_view label, int to_id, sf::Vector2<std::uint32_t> dim) : Animatable(svc, label), m_id{to_id}, m_label{label}, IWorldPositionable{{}, dim} { m_editor = svc.is_editor(); }

std::unique_ptr<Entity> Entity::clone() const { return std::unique_ptr<Entity>(); }

void Entity::serialize(dj::Json& out) {
	out["id"] = m_id;
	out["position"][0] = get_grid_position().x;
	out["position"][1] = get_grid_position().y;
	out["dimensions"][0] = get_grid_dimensions().x;
	out["dimensions"][1] = get_grid_dimensions().y;
}

void Entity::unserialize(dj::Json const& in) { m_id = in["id"].as<int>(); }

void Entity::expose() {
	ImGui::Text("Category: %s", m_label.c_str());
	ImGui::Separator();
	ImGui::InputInt("Entity ID", &m_id);
	ImGui::Text("Position: (%i", get_grid_position().x);
	ImGui::SameLine();
	ImGui::Text(", %i)", get_grid_position().y);
	ImGui::Separator();
}

void Entity::set_position(sf::Vector2u to_position) { set_grid_position(to_position); }

auto Entity::contains_point(sf::Vector2u test) const -> bool {
	for (auto i{0u}; i < get_grid_dimensions().x; ++i) {
		for (auto j{0u}; j < get_grid_dimensions().y; ++j) {
			if (get_grid_position() + sf::Vector2u{i, j} == test) { return true; }
		}
	}
	return false;
}

void Entity::update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, [[maybe_unused]] player::Player& player) { tick(); }

void Entity::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	if (!m_editor) { return; }
	drawbox.setOutlineColor(colors::blue);
	drawbox.setOutlineThickness(-2.f);
	if (highlighted) {
		drawbox.setOutlineColor(colors::ui_white);
		drawbox.setOutlineThickness(2.f);
	}
	drawbox.setSize(get_f_grid_dimensions() * size);
	drawbox.setPosition(get_f_grid_position() * size + cam);
	Animatable::set_scale(constants::f_scale_vec * size / constants::f_cell_size);
	Animatable::set_position(get_f_grid_position() * size + cam);
	if (m_textured) { win.draw(*this); }
	win.draw(drawbox);
}

} // namespace fornani
