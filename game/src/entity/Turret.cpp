
#include <fornani/entities/world/Laser.hpp>
#include <fornani/entity/Turret.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani {

Turret::Turret(automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "turrets", constants::i_resolution_vec), m_rate{560} {
	unserialize(in);
	init();
}

Turret::Turret(automa::ServiceProvider& svc, int id, TurretType type, TurretPattern pattern, CardinalDirection dir) : Entity(svc, "turrets", id), m_type{type}, m_pattern{pattern}, m_direction{dir}, m_rate{560} { init(); }

void Turret::init() {
	set_texture_rect(sf::IntRect{{}, constants::i_resolution_vec});
	repeatable = true;
	m_rate.start();
	Animatable::set_parameters({4, 2, 18, -1});
	Animatable::center();
}

std::unique_ptr<Entity> Turret::clone() const { return std::make_unique<Turret>(*this); }

void Turret::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["type"] = static_cast<int>(m_type);
	out["pattern"] = static_cast<int>(m_pattern);
	out["direction"] = m_direction.as<int>();
}

void Turret::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	m_type = static_cast<TurretType>(in["type"].as<int>());
	m_pattern = static_cast<TurretPattern>(in["pattern"].as<int>());
	m_direction = CardinalDirection{in["direction"].as<int>()};
}

void Turret::expose() {
	Entity::expose();
	auto type = static_cast<int>(m_type);
	auto patt = static_cast<int>(m_pattern);
	auto dir = m_direction.as<int>();
	ImGui::SliderInt("Type", &type, 0, 1);
	ImGui::SliderInt("Pattern", &patt, 0, 2);
	ImGui::SliderInt("Direction", &dir, 0, 1);
	m_type = static_cast<TurretType>(type);
	m_pattern = static_cast<TurretPattern>(patt);
	m_direction = CardinalDirection{dir};
}

void Turret::update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, [[maybe_unused]] player::Player& player) {
	Entity::update(svc, map, console, player);
	Animatable::set_rotation(sf::degrees(m_direction.as_degrees()));
	auto attributes = util::BitFlags<world::LaserAttributes>{};
	if (m_pattern == TurretPattern::constant) { attributes.set(world::LaserAttributes::infinite); }
	if (m_rate.just_started()) { map.lasers.push_back(world::Laser{svc, map, get_world_position() + constants::f_cell_vec.componentWiseMul(m_direction.as_vector()), world::LaserType::turret, attributes, m_direction, 256, 64}); }
	m_rate.update();
	if (m_rate.is_complete() && m_pattern != TurretPattern::constant) { m_rate.start(); }
}

void Turret::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{60, 255, 120, 180}) : drawbox.setFillColor(sf::Color{60, 255, 120, 80});
	Animatable::set_position(get_global_center() - cam);
	Entity::render(win, cam, size);
	if (m_editor) { return; }
	win.draw(*this);
}

} // namespace fornani
