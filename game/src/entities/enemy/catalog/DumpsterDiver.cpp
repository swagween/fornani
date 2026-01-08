
#include <fornani/entities/enemy/catalog/DumpsterDiver.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

DumpsterDiver::DumpsterDiver(automa::ServiceProvider& svc, world::Map& map, int variant) : Enemy(svc, map, "dumpster_diver"), m_services{&svc}, m_dive_time{800}, m_wait_time{400} {
	m_params = {{"idle", {0, 6, 48, -1}}};
	animation.set_params(get_params("idle"));
	p_state.actual = DumpsterDiverState::idle;
	m_wait_time.start();
	m_attack.hit.bounds.setRadius(64.f);

	m_variant = static_cast<DumpsterDiverVariant>(variant);

	get_collider().physics.set_friction_componentwise({0.995f, 0.99f});

	for (auto const& hp : map.home_points) {
		if (m_home_points.size() > 1) { continue; }
		m_home_points.push_back(hp);
	}
	std::sort(m_home_points.begin(), m_home_points.end(), [](sf::Vector2f const& a, sf::Vector2f const& b) { return a.x < b.x; });

	// determine surface points
	for (auto const& tile : map.get_middleground().get()->grid.cells) {
		if (!tile.exposed || !(tile.is_solid() || tile.is_ground_ramp())) { continue; }
		if (m_home_points.size() >= 2) {
			auto first = random::random_range(0, m_home_points.size() - 2);

			auto center = (m_home_points[first] + m_home_points[first + 1]) * 0.5f;
			auto radius = (m_home_points[first] - center).length();
			auto side = (tile.get_global_center() - m_home_points[0]).length() < (tile.get_global_center() - m_home_points[1]).length() ? 0 : 1;
			if (tile.bounding_box.overlaps_circle(center, radius) && !tile.bounding_box.overlaps_circle(center, radius - 128.f)) { m_surface_tiles.push_back({tile.get_global_center(), side}); }
		}
	}
}

void DumpsterDiver::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	m_dive_time.update();
	m_wait_time.update();
	if (m_wait_time.is_almost_complete()) { m_dive_time.start(); }
	flags.state.set(StateFlags::vulnerable);

	if (m_dive_time.is_almost_complete()) { set_flag(DumpsterDiverFlags::emerging, false); }

	// detect player
	static auto which_side = 0;
	if (!has_flag_set(DumpsterDiverFlags::emerging)) {
		for (auto const& pt : m_surface_tiles) {
			if (player.distant_vicinity.overlaps(pt.point)) {
				if (!m_wait_time.running() && !m_dive_time.running()) {
					m_wait_time.start();
					set_flag(DumpsterDiverFlags::emerging);
					which_side = pt.side;
				}
			}
		}
	}

	directions.desired = which_side == 0 ? Direction{{1, 0}} : Direction{{-1, 0}};
	if (directions.actual != directions.desired) { request_flip(); }

	// get two random sequential home points
	if (!m_wait_time.running()) {
		auto target_points = std::vector<sf::Vector2f>{};
		if (m_home_points.size() >= 2) {
			auto first = random::random_range(0, m_home_points.size() - 2);
			target_points = {m_home_points[first], m_home_points[first + 1]};
		}
		auto alpha = which_side == 1 ? m_dive_time.get_normalized() : m_dive_time.get_inverse_normalized();
		auto pos = util::arc_lerp_midpoint(target_points[0], target_points[1], alpha, 1.f, sf::Vector2f{0.f, 0.f});
		get_collider().set_position(pos);

		auto frame = std::floor(m_dive_time.get_inverse_normalized() * 6.f);
		Animatable::set_frame(frame);
	}

	if (svc.ticker.every_x_ticks(48)) {
		for (auto const& pt : m_surface_tiles) {
			if ((which_side == pt.side && m_wait_time.running()) || m_attack.hit.within_bounds(pt.point) || get_collider().bounding_box.overlaps(pt.point)) {
				map.spawn_emitter(svc, "mud", pt.point - sf::Vector2f{16.f, 16.f}, Direction{UND::up}, {32.f, 32.f});
			}
		}
	}

	// attack
	auto hit_offset = sf::Vector2f{directions.actual.as_float() * 64.f, -48.f};
	m_attack.hit.set_position(get_collider().get_center() + hit_offset);
	if (m_attack.hit.within_bounds(player.get_collider().hurtbox)) { player.hurt(); }

	// hurt
	if (flags.state.test(StateFlags::hurt)) {
		if (!hurt_effect.running()) { hurt_effect.start(128); }
		flags.state.reset(StateFlags::hurt);
	}

	state_function = state_function();
}

void DumpsterDiver::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (m_wait_time.running()) { return; }
	Enemy::render(svc, win, cam);
	if (svc.greyblock_mode()) {
		m_attack.render(win, cam);
		for (auto const& pt : m_surface_tiles) {
			sf::CircleShape test{};
			test.setPosition(pt.point - cam);
			test.setRadius(64.f);
			test.setFillColor({255, 255, 100, 60});
			win.draw(test);
		}
	}
	if (health.is_dead()) { return; }
}

fsm::StateFunction DumpsterDiver::update_idle() {
	p_state.actual = DumpsterDiverState::idle;
	return DUMPSTER_DIVER_BIND(update_idle);
}

bool DumpsterDiver::change_state(DumpsterDiverState next, anim::Parameters params) {
	if (p_state.desired == next) {
		animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
