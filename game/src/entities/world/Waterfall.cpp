
#include <fornani/entities/world/Waterfall.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::world {

Waterfall::Waterfall(automa::ServiceProvider& svc, Map& map, sf::Vector2u position) : Animatable(svc, "waterfall", {16, 16}), IWorldPositionable(position), m_bounding_box{get_world_dimensions()}, m_detector{get_world_dimensions()} {
	push_animation("main", {0, 8, 32, -1});
	set_animation("main");
	m_bounding_box.set_position(get_world_position());
	m_detector.set_position(get_world_position() + sf::Vector2f{0.f, 8.f});
}

void Waterfall::update(automa::ServiceProvider& svc, Map& map, player::Player& player) {
	tick();
	for (auto& water : map.get_entities<Water>()) {
		if (m_detector.overlaps(water->get_bounding_box())) {
			auto pos = m_detector.get_center() + sf::Vector2f{0.f, 16.f} + random::random_vector_float({-16.f, 0.f}, {16.f, 0.f});
			if (svc.ticker.every_x_ticks(18)) {
				map.spawn_effect(svc, "steam", pos, {random::random_range_float(-0.2f, 0.2f), -0.3f}, 0);
				map.spawn_emitter(svc, "steam", pos, Direction{UND::down});
			}
		}
	}
}

void Waterfall::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	m_bounding_box.render(win, cam, sf::Color{100, 160, 220, 30});
	// m_detector.render(win, cam, sf::Color{255, 0, 0, 60});
	Animatable::set_position(get_world_position() - cam);
	win.draw(*this);
}

} // namespace fornani::world
