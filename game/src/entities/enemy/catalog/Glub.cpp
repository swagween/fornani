
#include <fornani/entities/enemy/catalog/Glub.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

Glub::Glub(automa::ServiceProvider& svc, world::Map& map, int variant) : Enemy(svc, map, "glub", false, variant), m_services{&svc}, m_spawn{8} {
	p_animatable.set_animations({{"idle", {0, 2, 80, 2}}, {"turn", {0, 1, 80, 0}}});
	p_animatable.animation.set_params(get_params("idle"));
	p_state.actual = GlubState::idle;

	m_variant = static_cast<GlubVariant>(variant);
	flags.state.set(StateFlags::vulnerable);

	get_collider().physics.set_friction_componentwise({0.995f, 0.999f});
	flags.general.set(GeneralFlags::invincible_secondary);
	if (secondary_collider) { get_secondary_collider().set_dimensions({50.f, 50.f}); }

	m_spawn.start();

	for (int i = 0; i < 8; ++i) {
		auto const span = 220.f;
		auto const pos = sf::Vector2f{random::random_range_float(-span, span), random::random_range_float(-span, -50.f)};
		auto const link_length = 12.f;
		auto const chain_length = static_cast<int>(std::ceil(pos.length() / link_length));
		m_mines.push_back(MineChain{std::make_unique<entity::Mine>(svc, map, entity::MineType::floating),
									std::make_unique<vfx::Chain>(svc, "mine_chain", sf::Vector2i{5, 5}, vfx::SpringParameters{0.99f, 0.08f, 0.5f, 0.1f}, pos, chain_length, false, 2.f)});
		auto& me = m_mines.back();
		me.chain->set_num_angles(0);
		me.chain->flags.set(vfx::ChainFlags::no_collision);
		me.chain->flags.set(vfx::ChainFlags::linked);
		me.chain->parameters = vfx::ChainParameters{10.f, 1.1f, 10.f, 1.f, 12.f};
		me.offset = pos;
		me.timer = random::random_range_float(0.f, 1.f);
	}
}

void Glub::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (just_died()) { svc.soundboard.play_sound("beast_damage", get_collider().get_center()); }
	Enemy::update(svc, map, player);
	face_player(player);
	m_spawn.update();
	auto const center = get_collider().get_center();
	if (m_spawn.is_almost_complete()) {
		auto const adjustment = sf::Vector2f{random::coin_flip() ? -20.f : 20.f, 0.f};
		for (auto& m : m_mines) {
			auto mine_pos = center + m.offset;
			for (auto& e : map.enemy_catalog.enemies) {
				auto ctr = 0;
				while (e->get_collider().get_vicinity_rect().contains(mine_pos) && ctr < 100) {
					mine_pos += adjustment;
					++ctr;
				}
			}
			m.mine->set_position(mine_pos);
			m.chain->force_endpoints(mine_pos, center);
		}
	}

	for (auto& m : m_mines) {
		if (died() && m.timer > post_death.get_normalized()) { m.mine->explode(svc, map); }
		m.mine->update(svc, map, player);
		for (auto& proj : map.active_projectiles) { m.mine->on_hit(svc, map, proj, player); }
		m.chain->set_position(m.mine->get_collider().get_global_center());
		m.chain->set_end_position(center);
		m.chain->update(svc, map, player, 0.01f);
		if (m_spawn.is_almost_complete()) { m.chain->flags.reset(vfx::ChainFlags::linked); }
		if (m.mine->is_exploded() && !m.chain->flags.test(vfx::ChainFlags::broken)) {
			m.chain->set_free(true);
			m.chain->break_all();
		}
	}

	std::erase_if(m_mines, [](auto const& m) { return m.mine->is_exploded() && m.chain->is_destroyed(); });

	if (secondary_collider) {
		auto const yoff = 0.f;
		get_secondary_collider().set_position(center + (directions.actual.left() ? sf::Vector2f{-20.f - get_secondary_collider().dimensions.x * 0.5f, yoff} : sf::Vector2f{20.f - get_secondary_collider().dimensions.x * 0.5f, yoff}));
		get_secondary_collider().sync_components();
	}

	// hurt
	if (flags.state.test(StateFlags::hurt)) {
		m_services->soundboard.play_sound("hit_deep", center);
		if (!hurt_effect.running()) { hurt_effect.start(); }
		flags.state.reset(StateFlags::hurt);
	}

	if (directions.actual.lnr != directions.desired.lnr) { request(GlubState::turn); }
	state_function = state_function();
}

void Glub::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	for (auto& m : m_mines) {
		m.chain->render(svc, win, cam, false);
		m.mine->render(win, cam);
	}
	Enemy::render(svc, win, cam);
	if (health.is_dead()) { return; }
}

fsm::StateFunction Glub::update_idle() {
	p_state.actual = GlubState::idle;
	if (change_state(GlubState::turn, get_params("turn"))) { return GLUB_BIND(update_turn); }
	return GLUB_BIND(update_idle);
}

fsm::StateFunction Glub::update_turn() {
	p_state.actual = GlubState::turn;
	if (p_animatable.animation.complete()) {
		request_flip();
		request(GlubState::idle);
		if (change_state(GlubState::idle, get_params("idle"))) { return GLUB_BIND(update_idle); }
	}
	return GLUB_BIND(update_turn);
}

bool Glub::change_state(GlubState next, anim::Parameters params) {
	if (p_state.desired == next) {
		p_animatable.animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
