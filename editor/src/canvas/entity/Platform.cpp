
#include "editor/canvas/entity/Platform.hpp"

namespace pi {

Platform::Platform(dj::Json const& in) : Entity(in, "platforms") {
	unserialize(in);
	repeatable = true;
}
Platform::Platform(sf::Vector2u dim, int extent, std::string type, float start) : Entity("platforms", 0, dim), extent(extent), type(type), start(start) { repeatable = true; }

std::unique_ptr<Entity> Platform::clone() const { return std::make_unique<Platform>(*this); }

void Platform::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["extent"] = extent;
	out["type"] = type;
	out["start"] = start;
}

void Platform::unserialize(dj::Json const& in) { Entity::unserialize(in); }

void Platform::expose() {
	Entity::expose();
	ImGui::InputInt("Extent", &extent);
	ImGui::SliderFloat("start", &start, 0.f, 1.f, "%.3f");
}

void Platform::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{255, 255, 60, 100}) : drawbox.setFillColor(sf::Color{255, 255, 60, 40});
	Entity::render(win, cam, size);
}

} // namespace pi
