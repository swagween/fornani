
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/AirTravel.hpp>
#include <fornani/story/cutscene/Landing.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

AirTravel::AirTravel(automa::ServiceProvider& svc, world::Map& map) : Cutscene(svc, air_travel_id_v, "air_travel"), m_landed{200} {
	svc.input_system.flush_inputs();
	svc.camera_controller.set_owner(graphics::CameraOwner::player);
	svc.camera_controller.constrain();
	cooldowns.beginning.start();
	m_champion.emplace(svc, map);
	m_champion->set_channel(1);
	m_champion->get_collider().set_position(sf::Vector2f{80.f, 80.f});
	svc.events.travel_to_room_event.attach_to(p_slot, &AirTravel::travel_to_room, this);
	svc.soundboard.play_sound("radio_signal");
	auto const& in = svc.data.travel["landing_sites"][std::to_string(map.room_id)];
	if (svc.quest_table.get_quest_progression("landing_points", {in["tag"].as_string(), map.room_id}) < 1) { m_flags.set(AirTravelFlags::unavailable); }
	if (in["home_base"].as_bool()) { m_flags.set(AirTravelFlags::home_base); }
}

void AirTravel::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {

	auto const& in = svc.data.travel["landing_sites"][std::to_string(map.room_id)];

	if (m_flags.test(AirTravelFlags::unavailable)) {
		if (m_champion) { m_champion.reset(); }
		if (!context.console && !m_flags.test(AirTravelFlags::launched_console)) {
			context.console = std::make_unique<gui::Console>(svc, svc.text.basic, "no_radar_beacon", gui::OutputType::no_exit);
			m_flags.set(AirTravelFlags::launched_console);
		}
		if (progress == 40) {
			svc.quest_table.set_quest_progression("landing_points", {in["tag"].as_string(), map.room_id}, 1, {map.room_id});
			svc.soundboard.play_sound("pioneer_hard_slot");
			svc.soundboard.play_sound("pioneer_sync");
			for (auto& i : map.get_entities<Interactable>()) {
				if (i->get_tag() == "signal_beacon") { i->spawn(); }
			}
			++progress;
		}
		if (!context.console && progress == 41) { Cutscene::end(svc, player); }
		return;
	}

	auto npcs = map.get_entities<NPC>();
	auto const character = in["character"].as<int>();
	auto const convo = in["suite"].as<int>();
	auto const home_base = in["home_base"].as_bool();
	auto bit = std::ranges::find_if(npcs, [character](auto& n) { return n->get_specifier() == character; });
	auto& pilot = *bit;

	if (home_base && cooldowns.beginning.just_started()) { context.transition.start(); }
	if (m_flags.test(AirTravelFlags::home_base) && context.transition.is_black()) {
		m_flags.reset(AirTravelFlags::home_base);
		context.transition.end();
		pilot->unhide();
	}

	if (complete() && !m_flags.test(AirTravelFlags::done) && context.transition.is_black()) {
		m_flags.set(AirTravelFlags::done);
		pilot->hide();
		context.transition.end();
		Cutscene::end(svc, player);
		if (m_target_room) {
			svc.events.load_room_event.dispatch(svc, *m_target_room);
			svc.events.launch_cutscene_event.dispatch(svc, landing_id_v);
			player.get_collider().set_attribute(shape::ColliderAttributes::no_collision);
			player.get_collider().set_attribute(shape::ColliderAttributes::no_map_collision);
		}
	}

	if (m_flags.test(AirTravelFlags::done)) { return; }

	Cutscene::update(svc, context, map, player);

	m_landed.update();

	if (home_base && !m_flags.test(AirTravelFlags::home_base)) {
		for (auto& p : map.get_entities<AmbientProp>()) {
			if (p->get_tag() == "championj5") { p->set_hidden(true); }
			if (p->get_tag() == "totaled_championj5") { p->set_hidden(true); }
		}
	}

	if (m_champion) {
		if (svc.ticker.every_x_ticks(8)) { m_jitter = random::random_vector_float(-8.f, 8.f); }
		auto targetpos = map.get_random_home_point() - m_champion->get_collider().get_local_center() + m_jitter;
		if (progress > 21) { targetpos = {1000.f, -500.f}; }
		if (home_base) {
			targetpos = map.get_random_home_point() - m_champion->get_collider().get_local_center();
			m_champion->get_collider().set_position(targetpos);
			m_champion->flags.set(ChampionJ5Flags::interactable);
		}
		m_champion->set_target(targetpos);
		if (m_champion->is_close_to_target(8.f) && !m_flags.test(AirTravelFlags::started)) {
			if (player.get_actual_direction().right()) { player.turn(); }
			m_landed.start();
			m_flags.set(AirTravelFlags::started);
		}
		m_champion->update(svc, map);
		pilot->set_position(m_champion->get_drivers_seat());
		pilot->set_direction({LR::right});
		pilot->set_flag(NPCFlags::airborne);
		pilot->set_flag(NPCFlags::custom_camera);
		pilot->get_collider().set_attribute(shape::ColliderAttributes::no_map_collision);
		pilot->get_collider().set_attribute(shape::ColliderAttributes::no_collision);
	}

	if (context.console) { context.console.value()->set_no_exit(true); }

	if (npcs.empty()) { return; }

	auto going = progress < 6;
	if (context.console) { pilot->disengage(); }
	if (going) {
		pilot->set_special_animation(2);
		pilot->set_flag(NPCFlags::cutscene);
	}

	switch (progress) {
	case 0:
		if (!m_flags.test(AirTravelFlags::home_base)) { pilot->unhide(); }
		++progress;
		break;
	case 1: {
		if (!context.transition.is(graphics::TransitionState::inactive)) { break; }
		if (m_champion) {
			if (m_landed.is_almost_complete() || home_base) {
				pilot->flush_and_push(convo);
				pilot->force_engage();
				++progress;
			}
		}
		break;
	}
	case 2: {
		if (!context.console) {
			cooldowns.long_pause.start();
			++progress;
		}
		break;
	}
	case 3: {
		if (!context.console && !context.flags.test(SceneContextFlags::has_dialog) && !cooldowns.long_pause.running() && context.transition.is(graphics::TransitionState::inactive)) {
			pilot->flush_and_push(convo + 1);
			pilot->force_engage();
			++progress;
		}
		break;
	}
	case 10: {
		if (!context.console) { progress = 21; }
		break;
	}
	case 20: {
		if (!context.console) {
			pilot->flush_and_push(convo + 2);
			pilot->force_engage();
			cooldowns.pause.start();
			++progress;
		}
		break;
	}
	case 21: {
		if (!context.console && !cooldowns.pause.running() && !context.flags.test(SceneContextFlags::has_dialog) && context.transition.is(graphics::TransitionState::inactive)) {
			cooldowns.pause.start();
			++progress;
		}
		break;
	}
	case 22: {
		if (cooldowns.pause.is_almost_complete()) {
			cooldowns.end.start();
			context.transition.start();
			++progress;
		}
	} break;
	default: break;
	}
}

void AirTravel::render(sf::RenderWindow& win, sf::Vector2f cam) {
	if (m_champion && !m_flags.test(AirTravelFlags::home_base)) { m_champion->render(win, cam); }
	// debug_window();
}

void AirTravel::travel_to_room(int id) { m_target_room.emplace(id); }

} // namespace fornani
