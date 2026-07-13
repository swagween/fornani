
#include <fornani/entities/player/Player.hpp>
#include <fornani/entities/world/Laser.hpp>
#include <fornani/entity/Turret.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani {

Turret::Turret(automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "turrets", constants::i_resolution_vec), m_rate{560}, m_shoot{600} {
	unserialize(in);
	init(svc);
}

Turret::Turret(automa::ServiceProvider& svc, int id, TurretType type, TurretPattern pattern, CardinalDirection dir, TurretSettings settings)
	: Entity(svc, "turrets", id), m_type{type}, m_pattern{pattern}, m_direction{dir}, m_settings{settings}, m_rate{560} {
	init(svc);
}

void Turret::init(automa::ServiceProvider& svc) {
	p_animatable.set_texture_rect(sf::IntRect{{}, constants::i_resolution_vec});
	repeatable = true;
	auto start_time = m_pattern == TurretPattern::constant ? m_rate.get_native_time() : m_rate.get_native_time() * (1.f - m_settings.delay);
	m_rate.start(start_time);

	p_animatable.push_animation("off", {0, 1, 18, -1});
	p_animatable.push_animation("charging", {1, 3, 32, 0});
	p_animatable.push_animation("firing", {4, 2, 18, -1});
	p_animatable.push_animation("cooling_down", {6, 2, 18, 0});
	p_animatable.push_animation("quick_fire", {4, 4, 18, 0});

	m_firing = util::Cooldown{m_settings.duration};

	m_pattern == TurretPattern::constant ? p_animatable.set_animation("firing") : p_animatable.set_animation("off");

	m_position = get_world_position();

	if (m_type == TurretType::projectile) {
		m_weapon.emplace(std::make_unique<arms::Weapon>(svc, "turret", true));
		m_weapon.value()->set_team(arms::Team::skycorps);
	}
	if (m_type == TurretType::stun) {
		m_weapon.emplace(std::make_unique<arms::Weapon>(svc, "stungun", true));
		m_weapon.value()->set_team(arms::Team::skycorps);
	}
	auto label = m_type == TurretType::laser ? "turrets_laser" : "turrets_projectile";
	p_animatable.set_texture(svc.assets.get_texture(label));
}

std::unique_ptr<Entity> Turret::clone() const { return std::make_unique<Turret>(*this); }

void Turret::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["type"] = static_cast<int>(m_type);
	out["pattern"] = static_cast<int>(m_pattern);
	out["direction"] = m_direction.as<int>();
	out["settings"]["delay"] = m_settings.delay;
	out["settings"]["duration"] = m_settings.duration;
}

void Turret::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	m_type = static_cast<TurretType>(in["type"].as<int>());
	m_pattern = static_cast<TurretPattern>(in["pattern"].as<int>());
	m_direction = CardinalDirection{in["direction"].as<int>()};
	m_settings.delay = in["settings"]["delay"].as<float>();
	m_settings.duration = in["settings"]["duration"].as<int>();
}

void Turret::expose() {
	Entity::expose();
	auto type = static_cast<int>(m_type);
	auto patt = static_cast<int>(m_pattern);
	auto dir = m_direction.as<int>();
	ImGui::SliderInt("Type", &type, 0, 2);
	ImGui::SliderInt("Pattern", &patt, 0, 3);
	ImGui::SliderInt("Direction", &dir, 0, 1);
	ImGui::SliderFloat("Delay", &m_settings.delay, 0.0, 1.0, "%.1f");
	ImGui::SliderInt("Duration", &m_settings.duration, 8, 480);
	m_type = static_cast<TurretType>(type);
	m_pattern = static_cast<TurretPattern>(patt);
	m_direction = CardinalDirection{dir};
}

void Turret::update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] SceneContext& context, [[maybe_unused]] player::Player& player) {
	Entity::update(svc, map, context, player);
	m_firing.update();
	m_rate.update();
	m_shoot.update();

	if (m_type == TurretType::laser) { p_animatable.set_rotation(sf::degrees(m_direction.as_degrees())); }
	auto attributes = util::BitFlags<world::LaserAttributes>{};
	if (m_pattern == TurretPattern::constant) { attributes.set(world::LaserAttributes::infinite); }
	auto target = player.get_collider().get_center() - get_position();

	if (m_type == TurretType::laser) {
		if (m_firing.running() || m_pattern == TurretPattern::constant) {
			svc.soundboard.repeat_sound("laser_hum", get_handle(), get_global_center());
			svc.soundboard.repeat_sound("deep_laser_hum", get_handle(), get_global_center());
		}
	}
	if (m_type == TurretType::projectile || m_type == TurretType::stun) {
		m_rotator.handle_rotation(p_animatable.get_sprite(), target.perpendicular(), 4);
		p_animatable.set_channel(m_rotator.get_sprite_angle_index());
	}

	state_function = state_function();

	// opportunistic turrets
	if (m_pattern == TurretPattern::opportunistic && player.is_stunned() && !m_shoot.running()) {
		m_shoot.start();
		if (m_weapon) {
			map.spawn_projectile_at(svc, *m_weapon.value(), get_global_center(), target);
			svc.soundboard.play_sound("turret_shot", get_global_center());
		}
	}

	if (m_rate.just_started() && m_firing.is_complete()) {
		if (m_type == TurretType::stun) {
			if (m_weapon) {
				map.spawn_projectile_at(svc, *m_weapon.value(), get_global_center(), target);
				svc.soundboard.play_sound("stun_shot", get_global_center());
			}
		}
		if (m_type == TurretType::laser) { map.spawn_laser(svc, *this, get_position(), world::LaserType::turret, attributes, m_direction, m_settings.duration, 64, 0.75f); }
		m_firing.start();
	}
}

void Turret::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{60, 255, 120, 180}) : drawbox.setFillColor(sf::Color{60, 255, 120, 80});
	p_animatable.set_position(m_position + get_local_center() - cam);
	p_animatable.set_origin({});
	Entity::render(win, cam, size);
	if (m_editor) { return; }
	p_animatable.center();
	win.draw(p_animatable);
}

auto Turret::get_position() const -> sf::Vector2f { return m_position + get_local_center() + get_offset(); }

auto Turret::get_offset() const -> sf::Vector2f {
	auto offset = sf::Vector2f{};
	switch (m_direction.get()) {
	case UDLR::up: offset = {0.f, -0.5f * constants::f_cell_vec.y - constants::small_value}; break;
	case UDLR::down: offset = {0.f, 0.5f * constants ::f_cell_vec.y + constants::small_value}; break;
	case UDLR::left: offset = {-constants::f_cell_vec.x * 0.5f - constants::small_value, 0.f}; break;
	case UDLR::right: offset = {constants::f_cell_vec.x * 0.5f + constants::small_value, 0.f}; break;
	default: break;
	}
	return offset;
}

fsm::StateFunction Turret::update_off() {
	m_state.actual = TurretState::off;
	if (m_rate.is_almost_complete() && m_pattern != TurretPattern::opportunistic) { request(TurretState::charging); }
	if (m_pattern == TurretPattern::opportunistic && m_shoot.just_started()) { request(TurretState::quick_fire); }
	if (change_state(TurretState::charging, "charging")) { return TURRET_BIND(update_charging); }
	if (change_state(TurretState::firing, "firing")) { return TURRET_BIND(update_firing); }
	if (change_state(TurretState::quick_fire, "quick_fire")) { return TURRET_BIND(update_quick_fire); }
	return TURRET_BIND(update_off);
}

fsm::StateFunction Turret::update_charging() {
	m_state.actual = TurretState::charging;
	if (p_animatable.animation.is_complete()) {
		m_rate.start();
		request(TurretState::firing);
	}
	if (change_state(TurretState::firing, "firing")) { return TURRET_BIND(update_firing); }
	return TURRET_BIND(update_charging);
}

fsm::StateFunction Turret::update_firing() {
	m_state.actual = TurretState::firing;
	if (m_pattern == TurretPattern::constant) { m_firing.start(); }
	if (m_firing.is_almost_complete()) { request(TurretState::cooling_down); }
	if (change_state(TurretState::cooling_down, "cooling_down")) { return TURRET_BIND(update_cooling_down); }
	return TURRET_BIND(update_firing);
}

fsm::StateFunction Turret::update_cooling_down() {
	m_state.actual = TurretState::cooling_down;
	if (p_animatable.animation.is_complete()) { request(TurretState::off); }
	if (change_state(TurretState::off, "off")) { return TURRET_BIND(update_off); }
	return TURRET_BIND(update_cooling_down);
}

fsm::StateFunction Turret::update_quick_fire() {
	m_state.actual = TurretState::quick_fire;
	if (p_animatable.animation.is_complete()) { request(TurretState::off); }
	if (change_state(TurretState::off, "off")) { return TURRET_BIND(update_off); }
	return TURRET_BIND(update_cooling_down);
}

bool Turret::change_state(TurretState next, std::string_view tag) {
	if (m_state.desired == next) {
		p_animatable.set_animation(tag);
		return true;
	}
	return false;
}

} // namespace fornani
