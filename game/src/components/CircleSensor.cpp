
#include <fornani/components/CircleSensor.hpp>
#include <fornani/core/Debug.hpp>
#include <fornani/graphics/Colors.hpp>
#include <fornani/physics/CircleCollider.hpp>
#include <fornani/utils/Math.hpp>
#include <algorithm>

namespace fornani::components {

constexpr auto circle_vicinity_v = sf::Vector2f{80.f, 80.f};

CircleSensor::CircleSensor() : CircleSensor(32.f) {}

CircleSensor::CircleSensor(float radius) {
	bounds.setRadius(radius);
	bounds.setOutlineColor(sf::Color{255, 255, 255, 60});
	bounds.setOutlineThickness(-1.f);
	bounds.setFillColor(sf::Color::Transparent);
	bounds.setOrigin({radius, radius});
	drawable = bounds;
	m_vicinity = sf::FloatRect{{}, {sf::Vector2f{radius * 2.f, radius * 2.f} + circle_vicinity_v}};
}

void CircleSensor::render(sf::RenderWindow& win, sf::Vector2f cam) {
	drawable = bounds;
	drawable.setOrigin({bounds.getRadius(), bounds.getRadius()});
	drawable.setFillColor(active() ? sf::Color{20, 160, 160, 100} : sf::Color{160, 20, 160, 60});
	drawable.setPosition(bounds.getPosition() - cam);
	win.draw(drawable);
	if (debug::is_debug()) {
		sf::RectangleShape v{m_vicinity.size};
		v.setPosition(-cam);
		v.setFillColor(colors::transparent);
		v.setOutlineColor(colors::dark_orange);
		v.setOutlineThickness(-1.f);
		win.draw(v);
	}
}

void CircleSensor::set_position(sf::Vector2f position) {
	bounds.setPosition(position);
	m_vicinity.position = position - m_vicinity.size * 0.5f - bounds.getOrigin();
}

bool CircleSensor::within_bounds(sf::Vector2f const point) const { return (point - bounds.getPosition()).length() < bounds.getRadius(); }

bool CircleSensor::within_bounds(shape::Shape const& shape) const {
	if (shape.non_square()) { return shape.circle_SAT(bounds); }
	auto const x = std::clamp(bounds.getPosition().x, shape.get_position().x, shape.get_position().x + shape.get_dimensions().x);
	auto const y = std::clamp(bounds.getPosition().y, shape.get_position().y, shape.get_position().y + shape.get_dimensions().y);
	sf::Vector2 const closest{x, y};
	return util::magnitude(closest - bounds.getPosition()) < bounds.getRadius();
}

bool CircleSensor::within_bounds(shape::CircleCollider const& shape) const { return (bounds.getGlobalBounds().getCenter() - shape.get_global_center()).length() < bounds.getRadius() + shape.get_radius(); }

sf::Vector2f CircleSensor::get_overlap(shape::CircleCollider const& shape) const { return (bounds.getGlobalBounds().getCenter() - shape.get_global_center()).normalized(); }

sf::Vector2f CircleSensor::get_MTV(shape::Shape const& shape) const { return shape.circle_SAT_MTV(bounds); }

} // namespace fornani::components
