
#include <fornani/entities/packages/MobileProp.hpp>

namespace fornani {

MobileProp::MobileProp(automa::ServiceProvider& svc, world::Map& map, std::string_view tag, sf::Vector2i dimensions) : Mobile{svc, tag, dimensions} {
	register_collider(map, sf::Vector2f{dimensions * constants::i_scale_factor + sf::Vector2i{-4, -9}});
	get_collider().set_attribute(shape::ColliderAttributes::no_collision);
	get_collider().set_attribute(shape::ColliderAttributes::no_map_collision);
	p_animatable.center();
	friction = 0.995f;
}

void MobileProp::update(world::Map& map) {
	p_animatable.tick();
	get_collider().physics.set_friction_componentwise({friction, friction});
	if (has_flag_set(MobilePropFlags::dropped)) {
		get_collider().set_flag(shape::ColliderFlags::simple, false);
		get_collider().set_attribute(shape::ColliderAttributes::no_collision, false);
		get_collider().set_attribute(shape::ColliderAttributes::no_map_collision, false);
		get_collider().set_flag(shape::ColliderFlags::gravity, true);
		get_collider().physics.gravity = 2.f;
	} else {
		get_collider().set_flag(shape::ColliderFlags::simple);
		m_steering.seek(get_collider().physics, m_target, 0.1f);
	}
}

void MobileProp::render(sf::RenderWindow& win, sf::Vector2f cam) {
	p_animatable.set_position(get_collider().get_center() - cam);
	win.draw(p_animatable);
	// debug();
}

void MobileProp::debug() {
	static auto sz = ImVec2{380.f, 250.f};
	ImGui::SetNextWindowSize(sz);
	if (ImGui::Begin("Mobile Prop Debug")) {
		ImGui::Text("Position: (%.2f, %.2f)", get_collider().get_position().x, get_collider().get_position().y);
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
