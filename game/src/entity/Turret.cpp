
#include <fornani/entities/world/Laser.hpp>
#include <fornani/entity/Turret.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani {

Turret::Turret(automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "turrets"), m_rate{560} {
	unserialize(in);
	set_texture_rect(sf::IntRect{{}, constants::i_resolution_vec});
	repeatable = true;
}

Turret::Turret(automa::ServiceProvider& svc, int id, TurretType type, HV dir) : Entity(svc, "turrets", id), m_type{type}, m_direction{dir}, m_rate{560} {
	set_texture_rect(sf::IntRect{{}, constants::i_resolution_vec});
	repeatable = true;
}

std::unique_ptr<Entity> Turret::clone() const { return std::make_unique<Turret>(*this); }

void Turret::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["type"] = static_cast<int>(m_type);
	out["direction"] = static_cast<int>(m_direction);
}

void Turret::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	m_type = static_cast<TurretType>(in["type"].as<int>());
	m_direction = static_cast<HV>(in["direction"].as<int>());
}

void Turret::expose() {
	Entity::expose();
	auto type = static_cast<int>(m_type);
	auto dir = static_cast<int>(m_direction);
	ImGui::SliderInt("Type", &type, 0, 1);
	ImGui::SliderInt("Direction", &dir, 0, 1);
	m_type = static_cast<TurretType>(type);
	m_direction = static_cast<HV>(dir);
}

void Turret::update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, [[maybe_unused]] player::Player& player) {
	Entity::update(svc, map, console, player);
	if (m_rate.just_started()) {
		map.lasers.push_back(world::Laser{svc, map, get_world_position(), world::LaserType::turret, m_direction, 256, 64});
		NANI_LOG_DEBUG(m_logger, "Turret fired!");
	}
	m_rate.update();
	if (m_rate.is_complete()) { m_rate.start(); }
}

void Turret::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{60, 255, 120, 180}) : drawbox.setFillColor(sf::Color{60, 255, 120, 80});
	Entity::render(win, cam, size);
}

} // namespace fornani
