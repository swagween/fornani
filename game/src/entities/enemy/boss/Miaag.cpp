
#include <fornani/entities/enemy/boss/Miaag.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

static bool b_miaag_start{};
constexpr sf::Vector2f anchor_position_v{23.f, 22.f}; // where on the level the chain is connected
static void miaag_start_battle(int battle) { b_miaag_start = true; }
auto constexpr miaag_floor_destructibles = 50901;
auto constexpr miaag_outer_destructibles = 50902;
auto constexpr miaag_exit_destructibles = 509;

Miaag::Miaag(automa::ServiceProvider& svc, world::Map& map)
	: Boss(svc, map, "miaag"), m_magic{svc, "demon_magic"}, m_services{&svc}, m_map{&map}, m_cooldowns{.fire{48}, .charge{320}, .limit{960}, .post_magic{800}, .interlude{1000}, .chomped{800}, .post_death{1600}},
	  m_spine_sprite{svc.assets.get_texture("miaag_spines")}, m_spine{std::make_unique<vfx::Chain>(svc, vfx::SpringParameters{0.99f, 0.08f, 1.f, 4.f}, anchor_position_v * constants::f_cell_size, 8, false)} {
	m_params = {{"idle", {0, 7, 40, -1}}, {"chomp", {7, 9, 20, 0}},	   {"spellcast", {7, 5, 80, 0, true}}, {"hurt", {9, 1, 1000, 0}},
				{"turn", {16, 1, 40, 0}}, {"closed", {15, 1, 40, -1}}, {"awaken", {17, 4, 40, 0}},		   {"dormant", {17, 1, 40, -1}}};

	Enemy::animation.set_params(get_params("dormant"));
	svc.events.register_event(std::make_unique<Event<int>>("StartBattle", &miaag_start_battle));
	m_magic.set_team(arms::Team::guardian);
	flags.general.set(GeneralFlags::custom_channels);
	flags.general.set(GeneralFlags::post_death_render);
	Enemy::get_collider().set_flag(shape::ColliderFlags::simple);
	auto home = random::random_range(0, map.home_points.size() - 1);
	m_target_point = map.home_points.at(home);
	m_spine_sprite.setScale(constants::f_scale_vec);
	m_spine_sprite.setOrigin({40.5f, 26.f});
	for (auto const& link : m_spine->links) { m_spine_lookups.push_back(random::random_range(0, 2)); }
	m_spine->set_free(true);
}

void Miaag::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (m_cooldowns.post_death.is_almost_complete()) {
		svc.data.switch_destructible_state(miaag_exit_destructibles);
		svc.quest_table.progress_quest("defeat_miaag", 1, 509);
		svc.music_player.play_looped();
		svc.quest_table.set_quest_progression("npc_dialogue", {"dr_willett", 300}, 2, {300, 509}, 2);
	}

	Enemy::update(svc, map, player);
	face_player(player);

	m_cooldowns.fire.update();
	m_cooldowns.charge.update();
	m_cooldowns.limit.update();
	m_cooldowns.post_magic.update();
	m_cooldowns.interlude.update();
	m_cooldowns.chomped.update();
	m_cooldowns.post_death.update();

	m_magic.update(svc, map, *this);
	m_player_target = player.get_collider().get_center() + sf::Vector2f{0.f, -80.f};
	p_health_bar.update(health.get_normalized());
	m_spine->set_end_position(get_collider().get_center());
	m_spine->update(svc, map, player);

	if (half_health()) {
		if (!second_phase()) {
			set_flag(BossFlags::second_phase);
			m_cooldowns.interlude.start();
			svc.soundboard.flags.world.set(audio::World::vibration);
			svc.soundboard.flags.miaag.set(audio::Miaag::growl);
			svc.music_player.stop();
			svc.camera_controller.shake(10, 0.8f, 200, 20);
		}
	}
	if (m_cooldowns.interlude.is_almost_complete() && !player.is_dead()) {
		svc.data.switch_destructible_state(miaag_floor_destructibles);
		svc.music_player.load(svc.finder, "strife");
		svc.soundboard.flags.miaag.set(audio::Miaag::growl);
		svc.music_player.play_looped();
	}

	// movement
	auto movement_force = m_cooldowns.charge.running() ? 0.00005f : 0.0001f;
	if (battle_mode()) { m_steering.seek(Enemy::get_collider().physics, m_target_point + random::random_vector_float(-64.f, 64.f), movement_force); }

	if (directions.actual.lnr != directions.desired.lnr) { request(MiaagState::turn); }
	// attacks and animations
	if (battle_mode() && svc.ticker.every_x_ticks(1000) && !m_cooldowns.post_magic.running()) {
		if (second_phase()) {
			random::percent_chance(50) ? request(MiaagState::spellcast) : request(MiaagState::chomp);
		} else {
			request(MiaagState::spellcast);
		}
		was_requested(MiaagState::chomp) ? svc.soundboard.flags.miaag.set(audio::Miaag::growl) : svc.soundboard.flags.miaag.set(audio::Miaag::roar);
		auto home = random::random_range(0, map.home_points.size() - 1);
		m_target_point = map.home_points.at(home);
	}

	// effects
	if (m_cooldowns.interlude.running()) {
		if (m_cooldowns.interlude.get() % 32 == 0) {
			for (auto& destructible : map.destructibles) {
				if (destructible->get_id() == miaag_floor_destructibles) {
					map.effects.push_back(entity::Effect(*m_services, "puff", destructible->get_global_center(), {0.f, -0.2f}, 2));
					map.effects.back().random_start();
				}
			}
		}
		request(MiaagState::hurt);
	}
	if (health.is_dead() && !m_flags.test(MiaagFlags::gone)) {
		request(MiaagState::hurt);
		if (battle_mode()) {
			m_services->camera_controller.shake(10, 0.8f, 200, 20);
			m_services->soundboard.flags.world.set(audio::World::vibration);
			m_services->soundboard.flags.miaag.set(audio::Miaag::growl);
			m_services->music_player.stop();
			m_services->music_player.load(m_services->finder, "ritual");
			m_services->data.switch_destructible_state(miaag_floor_destructibles);
			m_services->data.switch_destructible_state(miaag_outer_destructibles);
			set_flag(BossFlags::battle_mode, false);
		}
	}
	if (health.is_dead()) {
		post_death.start(afterlife);
		flags.state.set(StateFlags::special_death_mode);
	}

	if (flags.state.test(StateFlags::hurt)) {
		if (!hurt_effect.running()) { hurt_effect.start(128); }
		svc.soundboard.flags.miaag.set(audio::Miaag::hurt);
		flags.state.reset(StateFlags::hurt);
	}
	auto flash_rate = 32;
	if (!is_state(MiaagState::spellcast)) { set_channel(EnemyChannel::standard); }
	if (hurt_effect.running()) { set_channel((hurt_effect.get() / flash_rate) % 2 == 0 ? EnemyChannel::hurt_1 : EnemyChannel::hurt_2); }

	if (b_miaag_start) {
		set_flag(BossFlags::battle_mode);
		svc.data.switch_destructible_state(miaag_outer_destructibles);
		request(MiaagState::awaken);
		b_miaag_start = false;
		svc.music_player.load(svc.finder, "scuffle");
		svc.music_player.play_looped();
		svc.soundboard.flags.miaag.set(audio::Miaag::hiss);
	}
	state_function = state_function();
}

void Miaag::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (m_flags.test(MiaagFlags::gone)) { return; }
	if (battle_mode()) {
		for (auto [i, link] : std::views::enumerate(m_spine->links)) {
			m_spine_sprite.setTextureRect(sf::IntRect{{0, 52 * m_spine_lookups[i]}, {81, 52}});
			m_spine_sprite.setPosition(link.get_bob() - cam);
			win.draw(m_spine_sprite);
		}
	}
	Enemy::render(svc, win, cam);
	if (svc.greyblock_mode()) { m_spine->render(svc, win, cam); }
}

void Miaag::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) { Boss::gui_render(svc, win, cam); }

fsm::StateFunction Miaag::update_dormant() {
	p_state.actual = MiaagState::dormant;
	if (change_state(MiaagState::awaken, get_params("awaken"))) { return MIAAG_BIND(update_awaken); }
	return MIAAG_BIND(update_dormant);
}

fsm::StateFunction Miaag::update_awaken() {
	p_state.actual = MiaagState::awaken;
	if (animation.complete()) {
		request(MiaagState::idle);
		flags.state.set(StateFlags::vulnerable);
		if (change_state(MiaagState::idle, get_params("idle"))) { return MIAAG_BIND(update_idle); }
	}
	return MIAAG_BIND(update_awaken);
}

fsm::StateFunction Miaag::update_idle() {
	p_state.actual = MiaagState::idle;
	if (change_state(MiaagState::hurt, get_params("hurt"))) { return MIAAG_BIND(update_hurt); }
	if (change_state(MiaagState::spellcast, get_params("spellcast"))) { return MIAAG_BIND(update_spellcast); }
	if (change_state(MiaagState::chomp, get_params("chomp"))) { return MIAAG_BIND(update_chomp); }
	if (change_state(MiaagState::turn, get_params("turn"))) { return MIAAG_BIND(update_turn); }
	return MIAAG_BIND(update_idle);
}

fsm::StateFunction Miaag::update_hurt() {
	p_state.actual = MiaagState::hurt;
	if (!m_flags.test(MiaagFlags::gone)) {
		if (m_services->ticker.every_x_ticks(32)) { m_map->effects.push_back(entity::Effect(*m_services, "large_explosion", get_collider().get_center() + random::random_vector_float(-80.f, 80.f), {}, 2)); }
		shake();
	}
	if (animation.complete()) {
		if (health.is_dead() && !m_flags.test(MiaagFlags::gone)) {
			m_cooldowns.post_death.start();
			m_flags.set(MiaagFlags::gone);
		} else {
			request(MiaagState::idle);
			if (change_state(MiaagState::idle, get_params("idle"))) { return MIAAG_BIND(update_idle); }
		}
	}
	return MIAAG_BIND(update_hurt);
}

fsm::StateFunction Miaag::update_closed() {
	p_state.actual = MiaagState::closed;
	return MIAAG_BIND(update_closed);
}

fsm::StateFunction Miaag::update_spellcast() {
	p_state.actual = MiaagState::spellcast;
	if (change_state(MiaagState::hurt, get_params("hurt"))) { return MIAAG_BIND(update_hurt); }
	if (animation.just_started()) {
		m_cooldowns.charge.start();
		m_cooldowns.limit.start();
		m_services->soundboard.flags.summoner.set(audio::Summoner::summon);
	}
	if (m_cooldowns.charge.is_complete()) {
		auto first_channel = m_cooldowns.fire.get() > 32;
		auto second_channel = m_cooldowns.fire.get() > 16;
		if (first_channel) {
			set_channel(EnemyChannel::extra_1);
		} else if (second_channel) {
			set_channel(EnemyChannel::extra_2);
		} else {
			set_channel(EnemyChannel::standard);
		}
		if (m_cooldowns.fire.is_complete()) {
			m_magic.get().set_barrel_point(get_collider().get_center());
			m_magic.shoot(*m_services, *m_map, m_player_target - get_collider().get_center());
			m_services->soundboard.flags.weapon.set(audio::Weapon::demon_magic);
			m_cooldowns.fire.start();
		}
	}
	if (m_services->ticker.every_x_ticks(32)) { m_map->effects.push_back(entity::Effect(*m_services, "demon_breath", get_collider().get_center() + random::random_vector_float(-40.f, 40.f), {0.f, -0.2f})); }
	if (m_cooldowns.limit.is_almost_complete()) {
		request(MiaagState::idle);
		set_channel(EnemyChannel::standard);
		m_cooldowns.post_magic.start();
		if (change_state(MiaagState::idle, get_params("idle"))) { return MIAAG_BIND(update_idle); }
	}
	return MIAAG_BIND(update_spellcast);
}

fsm::StateFunction Miaag::update_chomp() {
	p_state.actual = MiaagState::chomp;
	if (change_state(MiaagState::hurt, get_params("hurt"))) { return MIAAG_BIND(update_hurt); }
	if (animation.get_frame_count() == 5 && !m_cooldowns.chomped.running()) {
		for (auto i{0}; i < 4; ++i) { m_map->spawn_enemy(18, get_collider().get_center() + random::random_vector_float(-180.f, 180.f), 2); }
		m_services->soundboard.flags.miaag.set(audio::Miaag::chomp);
		m_cooldowns.chomped.start();
	}
	if (animation.is_complete()) {
		request(MiaagState::idle);
		set_channel(EnemyChannel::standard);
		m_cooldowns.post_magic.start();
		if (change_state(MiaagState::idle, get_params("idle"))) { return MIAAG_BIND(update_idle); }
	}
	return MIAAG_BIND(update_chomp);
}

fsm::StateFunction Miaag::update_turn() {
	p_state.actual = MiaagState::turn;
	if (change_state(MiaagState::hurt, get_params("hurt"))) { return MIAAG_BIND(update_hurt); }
	if (change_state(MiaagState::spellcast, get_params("spellcast"))) { return MIAAG_BIND(update_spellcast); }
	if (change_state(MiaagState::chomp, get_params("chomp"))) { return MIAAG_BIND(update_chomp); }
	directions.desired.lock();
	if (animation.complete()) {
		request_flip();
		request(MiaagState::idle);
		animation.set_params(get_params("idle"));
		return MIAAG_BIND(update_idle);
	}
	return MIAAG_BIND(update_turn);
}

bool Miaag::change_state(MiaagState next, anim::Parameters params) {
	if (p_state.desired == next) {
		Enemy::animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
