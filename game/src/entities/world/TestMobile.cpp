
#include <fornani/entities/world/TestMobile.hpp>

namespace fornani {

TestMobile::TestMobile(automa::ServiceProvider& svc, world::Map& map) : Mobile{svc, "test_collider", {16, 16}} {
	register_collider(map, {32.f, 32.f});
	get_collider().set_attribute(shape::ColliderAttributes::no_collision);
	push_and_set_animation("basic", {0, 1, 32, -1});
	center();
	friction = 1.f;
}

void TestMobile::update(world::Map& map) {
	tick();
	get_collider().set_attribute(shape::ColliderAttributes::no_map_collision);
	get_collider().set_flag(shape::ColliderFlags::simple);
	get_collider().physics.set_friction_componentwise({friction, friction});
	m_steering.thrust_seek(get_collider().physics, m_target, thrust_params);
}

void TestMobile::render(sf::RenderWindow& win, sf::Vector2f cam) {
	set_position(get_collider().get_center() - cam);
	win.draw(*this);
	debug();
}

void TestMobile::debug() {
	static auto sz = ImVec2{380.f, 250.f};
	ImGui::SetNextWindowSize(sz);
	if (ImGui::Begin("Test Mobile Debug")) {
		ImGui::DragFloat("friction", &friction, 0.0001f);
		ImGui::DragFloat("thrust power", &thrust_params.thrust_power, 0.0001f);
		ImGui::DragFloat("thrust daming", &thrust_params.damping, 0.0001f);
		ImGui::DragFloat("thrust turn rate", &thrust_params.turn_rate, 0.001f);
		ImGui::DragFloat("arrival radius", &thrust_params.arrival_radius, 10.f);
		if (ImGui::Button("left")) { m_target = {100.f, 100.f}; }
		if (ImGui::Button("right")) { m_target = {500.f, 200.f}; }
		ImGui::End();
	}
}

} // namespace fornani
