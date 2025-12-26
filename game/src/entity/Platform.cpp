
#include <fornani/entity/Platform.hpp>
#include <fornani/graphics/Colors.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Constants.hpp>

namespace fornani {

Platform::Platform(fornani::automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "platforms") {
	unserialize(in);
	init(svc);
}

Platform::Platform(fornani::automa::ServiceProvider& svc, sf::Vector2u dim, int extent, std::string type, float start) : Entity(svc, "platforms", 0, dim), extent(extent), type(type), start(start) { init(svc); }

std::unique_ptr<Entity> Platform::clone() const { return std::make_unique<Platform>(*this); }

void Platform::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["extent"] = extent;
	out["type"] = type;
	out["start"] = start;
}

void Platform::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	extent = in["extent"].as<int>();
	type = in["type"].as_string();
	start = in["start"].as<float>();
}

void Platform::expose() {
	Entity::expose();
	ImGui::InputInt("Extent", &extent);
	ImGui::SliderFloat("start", &start, 0.f, 1.f, "%.3f");
}

void Platform::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{255, 255, 60, 100}) : drawbox.setFillColor(sf::Color{255, 255, 60, 40});
	m_track.setScale(sf::Vector2f{size, size} / fornani::constants::f_cell_size);
	m_track.setPosition(get_f_grid_position() * size + cam + get_f_grid_dimensions() * size * 0.5f);
	win.draw(m_track);
	Entity::render(win, cam, size);
	win.draw(m_track);
}

void Platform::init(fornani::automa::ServiceProvider& svc) {
	repeatable = true;
	stackable = true;
	auto offset = sf::Vector2i{};
	if (get_grid_dimensions().x == 1) { offset = {0, 0}; }
	if (get_grid_dimensions().x == 2) { offset = {16, 0}; }
	if (get_grid_dimensions().x == 3) { offset = {0, 16}; }
	if (get_grid_dimensions().y == 2) { offset = {0, 32}; }
	if (get_grid_dimensions().y == 3) { offset = {0, 64}; }
	set_texture_rect(sf::IntRect{{offset}, {sf::Vector2i{get_grid_dimensions()} * fornani::constants::i_cell_resolution}});
	m_track.setFillColor(sf::Color::Transparent);
	auto color = fornani::colors::goldenrod;
	color.a = 80;
	m_track.setOutlineColor(color);
	m_track.setOutlineThickness(2.f);
	auto const& in_data = svc.data.platform[type];
	auto bounds = sf::Vector2f{};
	for (auto const& point : in_data["track"].as_array()) {
		bounds.x = std::max(bounds.x, point[0].as<float>());
		bounds.y = std::max(bounds.y, point[1].as<float>());
	}
	m_track.setSize(bounds * static_cast<float>(extent));
}

} // namespace fornani
