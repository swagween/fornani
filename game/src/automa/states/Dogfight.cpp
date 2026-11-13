
#include <fornani/automa/states/Dogfight.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/entity/NPC.hpp>
#include <fornani/events/GameplayEvent.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::automa {

Dogfight::Dogfight(ServiceProvider& svc, player::Player& player, int room_number)
	: GameplayState(svc, player, room_number), m_background{svc, "cloud_sea", {1024, 512}}, m_intro_shot{10}, m_bryn_shoot{1000}, m_bryn_fire{40}, m_nimbus_arrive{3000} {

	m_type = StateType::dojo;

	m_map.emplace(svc, player);
	reload(svc, room_number);

	svc.data.save_progress(player, dogfight_id_v);
}

void Dogfight::init(ServiceProvider& svc, player::Player& player) {
	svc.app_flags.set(automa::AppFlags::in_game);

	m_controller = {};
	m_world_position = {};
	m_enemy_counter.start();
	player.start_over();

	svc.music_player.load(svc.finder, "scuffle");
	svc.ambience_player.load(svc.finder, "firstwind");
	svc.ambience_player.play();

	player.reset_flags();
	player.set_dogfight();
	player.set_flag(player::PlayerFlags::dogfight);
	svc.world_clock.set_rng(WorldClockInterval::day, 0.9f);
	svc.world_clock.set_time(10, 20);

	m_map->set_no_collision(true);

	auto npcs = m_map->get_entities<NPC>();
	auto bit = std::ranges::find_if(npcs, [](auto& n) { return n->get_specifier() == 35; });
	if (bit != npcs.end()) {
		m_bryn = *bit;
		m_bryn->unhide();
		m_bryn->set_flag(NPCFlags::airborne);
		m_bryn->set_flag(NPCFlags::custom_requests);
		m_bryn->set_animation("looping_1");
	}

	auto const bg = player.has_weapon("wasp");
	m_bryns_gun.emplace(svc, bg ? "bryns_gun" : "wasp");

	m_champion.emplace(svc, *m_map);
	auto const start = sf::Vector2f{600.f, 256.f};
	m_champion->set_position(start);
	m_controller.set_position(start);
	m_champion->flags.set(ChampionJ5Flags::controlled);
	m_champion->set_battle(true);

	svc.soundboard.turn_off();
	svc.app_flags.reset(automa::AppFlags::custom_map_start);

	p_world_shader = LightShader(svc.finder);
	svc.a11y.set_action_ctx_bar_enabled(false);
	p_world_shader->set_darken(m_map->get_darken_factor());
	p_world_shader->set_texture_size(m_map->real_dimensions / constants::f_scale_factor);
	p_entity_shader = LightShader(svc.finder);

	m_intro_shot.start();
	m_bryn_shoot.start();
	m_nimbus_arrive.start();

	svc.ambience_player.volume.set_dynamic(0.0);
}

void Dogfight::tick_update(ServiceProvider& svc, capo::IEngine& engine) {

	svc.soundboard.set_listener_position(player->get_ear_position());

	set_flag(GameplayStateFlags::early_tick_return, false);
	GameplayState::tick_update(svc, engine);
	if (has_flag_set(GameplayStateFlags::early_tick_return)) { return; }

	player->set_flag(player::PlayerFlags::console_open, p_context.console.has_value());
	if (p_context.console) {
		p_flags.set(GameplayFlags::console_running);
		if (p_context.console.value()->was_response_created() && !p_context.console.value()->has_nani_portrait()) {
			player->wardrobe_widget.update(*player);
			p_context.console.value()->set_nani_sprite(player->wardrobe_widget.get_sprite());
		}
	} else if (p_flags.consume(GameplayFlags::console_running)) {
		auto to_set = p_inventory_window || p_dialog ? input::ActionSet::Menu : input::ActionSet::Platformer;
		svc.input_system.set_action_set(to_set);
	}

	svc.world_clock.update(svc);

	if (p_inventory_window) {
		m_map->set_target_balance(0.f, audio::BalanceTarget::music);
		m_map->set_target_balance(0.f, audio::BalanceTarget::ambience);
		m_map->update_balance(svc);
		if (!p_context.console) {
			p_inventory_window.value()->update(svc, *player, *m_map);
			if (p_inventory_window.value()->exit_requested()) {
				p_inventory_window.reset();
				svc.input_system.set_action_set(input::ActionSet::Platformer);
			}
			return;
		}
	}

	m_intro_shot.update();
	m_bryn_shoot.update();
	m_bryn_fire.update();
	m_nimbus_arrive.update();

	if (!m_map) { return; }

	for (auto& e : m_map->enemy_catalog.enemies) { e->set_reference_point(m_world_position); }
	if (svc.ticker.every_x_ticks(1400) && m_map->spawn_counter.get_count() < 2) {
		m_map->spawn_enemy(51, m_world_position + sf::Vector2f{-100.f, random::range_float(0.f, 960.f)}, 0, true, false, true);
		m_enemy_counter.update();
	}

	if (m_nimbus_arrive.is_almost_complete()) { m_map->spawn_enemy(52, m_world_position + sf::Vector2f{-160.f, random::range_float(0.f, 960.f)}, 0, true, false, true); }

	if (m_champion) {
		for (auto& proj : m_map->active_projectiles) { m_champion->on_hit(proj, *m_map); }
		m_champion->update(svc, *m_map);
	}
	if (!player->is_dead()) { m_controller.handle_input(svc.input_system); }
	m_controller.update(svc);
	if (m_champion) {
		if (m_champion->is_dead() && !player->is_dead()) { player->set_death_type(player::PlayerDeathType::shot_down); }
		m_champion->get_collider().physics.velocity = m_controller.get_physics().velocity;
		m_champion->set_position(m_controller.get_position() + m_world_position);
		// if (m_controller.is_dashing() && svc.ticker.every_x_ticks(16)) { m_map->spawn_effect(svc, "smoke", m_champion->get_collider().get_center()); }
	}

	auto const speed = 2.5f;
	m_world_position.x += speed;
	if (player->is_dead()) {
		if (svc.ticker.every_x_ticks(24)) { m_map->spawn_effect(svc, "explosion", m_champion->get_collider().get_center() + random::vector_float(-30.f, 30.f), {}, 3); }
	}

	if (m_flags.test(DogfightFlags::complete) && p_context.cutscene_catalog.cutscenes.empty()) {
		svc.state_controller.actions.set(automa::Actions::trigger);
		svc.app_flags.set(automa::AppFlags::custom_map_start);
		player->set_sleeping();
		player->set_sleep_timer();
		player->set_direction(Direction{{1, 0}});
	}

	svc.a11y.set_action_ctx_bar_enabled(false);

	if (p_context.console) {
		if (p_context.console.value()->was_response_created() && !p_context.console.value()->has_nani_portrait()) {
			player->wardrobe_widget.update(*player);
			p_context.console.value()->set_nani_sprite(player->wardrobe_widget.get_sprite());
		}
	}

	svc.world_clock.update(svc);

	// physical tick
	if (m_champion) {
		player->set_position(m_champion->get_passengers_seat());
		if (m_bryn) { m_bryn->set_position(m_champion->get_drivers_seat()); }
	}
	if (m_bryn) {
		if (m_bryn_shoot.is_almost_complete()) {
			m_bryn->request(NPCAnimationState::oneoff_1);
			m_bryn_shoot.start();
			m_bryn_fire.start();
		}
		if (m_bryns_gun) {
			m_bryns_gun->set_position(m_bryn->get_collider().get_center());
			m_bryns_gun->set_firing_direction(Direction{LR::left});
			m_bryns_gun->update(svc, *m_map, Direction{LR::left});
			if (m_bryn->is_state(NPCAnimationState::oneoff_1) && m_bryn_fire.is_almost_complete()) {
				m_bryns_gun->shoot(svc, *m_map);
				m_bryn_fire.start();
			}
		}
		if (m_intro_shot.running()) { m_bryn->request(NPCAnimationState::looping_1); }
	}
	player->update(*m_map);
	player->get_collider().physics.velocity = m_controller.get_physics().velocity + sf::Vector2f{speed, 0.f};
	player->start_tick();
	m_map->update(svc, p_context);
	handle_player_death(svc, *player);

	m_map->debug_mode = debug_mode;

	player->end_tick();
	if (!p_context.console) { player->set_busy(false); }

	m_map->background->update(svc);
	hud.update(svc, *player);
}

void Dogfight::frame_update(ServiceProvider& svc) { player->frame_update(); }

void Dogfight::render(ServiceProvider& svc, sf::RenderWindow& win) {
	if (!m_map) { return; }
	auto cam = m_world_position;
	p_renderer.begin(win, cam);
	m_background.render(svc, win, {});
	if (p_world_shader) {
		m_map->render_background(p_renderer, svc, win, p_world_shader, cam);
		p_entity_shader->set_texture_size(m_map->real_dimensions / constants::f_scale_factor);
		p_entity_shader->finalize(svc.data.biomes["properties"][m_map->get_biome_string()]["max_light"].as<float>());
		auto ent_sprite = sf::Sprite{m_map->m_entity_texture.getTexture()};
		ent_sprite.setPosition(-cam);
		if (m_palette) {
			p_entity_shader->submit(win, *m_palette, ent_sprite);
		} else {
			win.draw(ent_sprite);
		}
		auto sent_sprite = sf::Sprite{m_map->m_static_entity_texture.getTexture()};
		sent_sprite.setPosition(-cam);
		if (m_palette) { p_entity_shader->submit(win, *m_palette, sent_sprite); }
		if (m_champion) { m_champion->render(win, cam); }
		m_map->render(p_renderer, svc, win, p_world_shader, cam);
	}
	if (m_bryns_gun && m_bryn) {
		if (m_bryn->is_state(NPCAnimationState::oneoff_1)) { m_bryns_gun->render(svc, win, cam); }
	}

	if (m_champion) { m_champion->gui_render(win); }

	GameplayState::render(svc, win);

	// sf::CircleShape dot{4.f};
	// dot.setPosition(m_world_position + m_controller.get_target() - cam);
	//  win.draw(dot);

	// debug();
}

void Dogfight::reload(ServiceProvider& svc, int target_state) {
	GameplayState::reload(svc, target_state);

	set_flag(GameplayStateFlags::transitioned_in, false);
	p_flags.reset(GameplayFlags::transitioning);
	p_context.transition.set(graphics::TransitionState::black);
	p_flags.reset(GameplayFlags::death_console_launched);
	svc.state_controller.actions.reset(Actions::save_loaded);

	m_bryn = nullptr;
	m_bryns_gun.reset();
	m_champion.reset();

	init(svc, *player);
}

void Dogfight::handle_player_death(ServiceProvider& svc, player::Player& player) {
	if (!m_map) { return; }
	if (!player.is_dead()) { return; }

	m_controller.spiral();

	// check if player died
	if (!p_flags.test(GameplayFlags::game_over) && player.is_death_complete()) {
		p_flags.set(GameplayFlags::death_console_launched);
		svc.app_flags.reset(automa::AppFlags::in_game);
		p_context.console = std::make_unique<gui::Console>(svc, svc.text.basic, "death", gui::OutputType::no_exit, 8);
		p_flags.set(GameplayFlags::game_over);
		svc.music_player.load(svc.finder, "mortem");
		svc.music_player.play_looped();
		svc.soundboard.turn_off();
		player.visit_history.clear();
		svc.stats.player.death_count.update();
	}

	if (!p_context.console && p_flags.test(GameplayFlags::game_over)) {
		if (!p_flags.test(GameplayFlags::transitioning)) { p_context.transition.start(); }
		p_flags.set(GameplayFlags::transitioning);
		if (p_context.transition.is(graphics::TransitionState::black) && !svc.state_controller.actions.test(automa::Actions::player_death)) {
			svc.state_controller.actions.set(automa::Actions::player_death);
			p_flags.reset(GameplayFlags::game_over);
		}
	}
}

void Dogfight::debug() {
	if (ImGui::Begin("Dogfight Debug")) {
		auto const print_vec = [](char const* label, sf::Vector2f const& pos) {
			ImGui::Text("%s", label);
			ImGui::SameLine(140.f);
			ImGui::Text("%.3f, %.3f", pos.x, pos.y);
		};

		ImGui::Text("World position: X %.2f | Y %.2f", m_world_position.x, m_world_position.y);

		auto const controller_pos = m_controller.get_position();

		if (ImGui::CollapsingHeader("Controller", ImGuiTreeNodeFlags_DefaultOpen)) { print_vec("Position", controller_pos); }

		if (m_champion && ImGui::CollapsingHeader("Champion", ImGuiTreeNodeFlags_DefaultOpen)) {
			auto const champion_pos = m_champion->get_position();
			auto const seat_pos = m_champion->get_passengers_seat();
			auto const delta = champion_pos - controller_pos;

			print_vec("Position", champion_pos);
			print_vec("Passenger seat", seat_pos);
			ImGui::Separator();
			print_vec("Controller delta", delta);

			ImGui::Text("Distance: %.3f", std::hypot(delta.x, delta.y));
		}
		if (ImGui::Button("Restart")) { m_enemy_counter.start(); }
		if (m_champion) {
			ImGui::Separator();
			ImGui::TextDisabled("Position synchronization");

			auto const champion_pos = m_champion->get_position();
			auto const delta = champion_pos - controller_pos;

			ImGui::Text("Controller -> Champion");
			ImGui::SameLine();
			if (delta == sf::Vector2f{}) {
				ImGui::TextColored(ImVec4{0.3f, 0.9f, 0.3f, 1.f}, "SYNCED");
			} else {
				ImGui::TextColored(ImVec4{1.f, 0.7f, 0.2f, 1.f}, "OFFSET");
			}
		}
	}
	ImGui::End();
}

} // namespace fornani::automa
