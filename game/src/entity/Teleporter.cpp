
#include <fornani/entity/Teleporter.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani {

Teleporter::Teleporter(fornani::automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "teleporters", fornani::constants::i_resolution_vec_padded), m_sending{40}, m_receiving{40}, m_sensor{14.f} {
	unserialize(in);
	p_animatable.push_and_set_animation("basic", {0, 10, 12, -1});
	repeatable = true;
	m_sensor.set_position(get_world_position());
	p_animatable.set_origin({2.f, 2.f});
}

Teleporter::Teleporter(fornani::automa::ServiceProvider& svc, int id, int type, int dir) : Entity(svc, "teleporters", id), m_type{type}, m_direction{dir} {
	p_animatable.set_texture_rect(sf::IntRect{{}, fornani::constants::i_resolution_vec});
	repeatable = true;
}

std::unique_ptr<Entity> Teleporter::clone() const { return std::make_unique<Teleporter>(*this); }

void Teleporter::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["direction"] = m_direction.as<int>();
}

void Teleporter::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	m_direction = CardinalDirection{in["direction"].as<int>()};
}

void Teleporter::expose() {
	Entity::expose();
	static int dir{};
	ImGui::InputInt("Type", &m_type);
	ImGui::InputInt("Dir", &dir);
	m_direction = CardinalDirection{dir};
}

void Teleporter::update(automa::ServiceProvider& svc, world::Map& map, SceneContext& context, player::Player& player) {
	p_animatable.tick();
	m_sending.update();
	m_receiving.update();
	if (m_sending.running()) { return; }
	if (m_receiving.running()) { return; }
	for (auto& proj : map.active_projectiles) {
		if (m_sensor.within_bounds(proj.get_collider())) {
			if (map.has_entities()) {
				for (auto& t : map.get_entities<Teleporter>()) {
					if (t != this && t->get_id() == get_id()) {
						proj.set_position(t->get_global_center());
						proj.set_direction(t->get_direction());
						m_sending.start();
						t->receive();
					}
				}
			}
		}
	}
	for (auto& p : map.pushables) {
		if (m_sensor.within_bounds(p->get_bounding_box())) {
			if (map.has_entities()) {
				for (auto& t : map.get_entities<Teleporter>()) {
					if (t != this && t->get_id() == get_id()) {
						p->get_collider().set_position(t->get_world_position());
						if (t->get_direction().up()) {
							p->get_collider().physics.acceleration.y *= -2.f;
							p->get_collider().physics.velocity.y *= -2.f;
						}
						m_sending.start();
						t->receive();
					}
				}
			}
		}
	}
}

void Teleporter::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{60, 255, 120, 180}) : drawbox.setFillColor(sf::Color{60, 255, 120, 20});
	Entity::render(win, cam, size);
	p_animatable.set_position(get_world_position() - cam);
	if (m_editor) { return; }
	win.draw(p_animatable);
}

} // namespace fornani
