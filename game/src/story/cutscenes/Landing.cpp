
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/Landing.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

Landing::Landing(automa::ServiceProvider& svc, world::Map& map) : Cutscene(svc, landing_id_v, "landing"), m_landed{200} {
	svc.input_system.flush_inputs();
	m_champion.emplace(svc, map);
	m_champion->set_channel(1);
	m_champion->get_collider().set_position(sf::Vector2f{80.f, 120.f});
}

void Landing::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {

	auto npcs = map.get_entities<NPC>();
	auto const& in = svc.data.travel["landing_sites"][std::to_string(map.room_id)];
	auto const character = in["character"].as<int>();
	auto const convo = in["farewell"].as<int>();
	auto const home_base = in["home_base"].as_bool();
	auto bit = std::ranges::find_if(npcs, [character](auto& n) { return n->get_specifier() == character; });
	auto& pilot = *bit;

	if (home_base) {
		for (auto& p : map.get_entities<AmbientProp>()) {
			if (p->get_tag() == "championj5") { p->set_hidden(true); }
			if (p->get_tag() == "totaled_championj5") { p->set_hidden(true); }
		}
	}

	if (complete() && !m_flags.test(LandingFlags::done) && context.transition.is_black()) {
		m_flags.set(LandingFlags::done);
		pilot->hide();
		context.transition.end();
		Cutscene::end(svc, player);
		for (auto& p : map.get_entities<AmbientProp>()) {
			if (p->get_tag() == "championj5") { p->set_hidden(false); }
			if (p->get_tag() == "totaled_championj5") { p->set_hidden(false); }
		}
	}

	if (m_flags.test(LandingFlags::done)) { return; }

	Cutscene::update(svc, context, map, player);
	svc.camera_controller.set_owner(graphics::CameraOwner::player);
	svc.camera_controller.constrain();

	if (progress < 2) {
		player.get_collider().set_attribute(shape::ColliderAttributes::no_collision);
		player.get_collider().set_attribute(shape::ColliderAttributes::no_map_collision);
		if (m_champion) { player.set_position(m_champion->get_passengers_seat()); }
		player.set_sitting();
		player.set_direction({LR::right});
	} else {
		player.get_collider().set_attribute(shape::ColliderAttributes::no_collision, false);
		player.get_collider().set_attribute(shape::ColliderAttributes::no_map_collision, false);
	}

	m_landed.update();

	if (m_champion) {
		if (svc.ticker.every_x_ticks(8)) { m_jitter = random::random_vector_float(-8.f, 8.f); }
		auto targetpos = map.get_random_home_point() - m_champion->get_collider().get_local_center() + m_jitter - sf::Vector2f{0.f, 36.f};
		if (progress > 21) { targetpos = {1000.f, -500.f}; }
		m_champion->set_target(targetpos);
		if (m_champion->is_close_to_target(12.f) && !m_flags.test(LandingFlags::started)) {
			m_champion->flags.set(ChampionJ5Flags::interactable);
			m_landed.start();
			m_flags.set(LandingFlags::started);
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
		pilot->unhide();
		++progress;
		break;
	case 1: {
		if (m_champion) {
			if (m_landed.is_almost_complete()) {
				pilot->flush_and_push(convo);
				pilot->force_engage();
				++progress;
			}
		}
		break;
	}
	case 2: {
		if (!context.console) {
			cooldowns.end.start();
			context.transition.start();
			++progress;
		}
		break;
	}
	default: break;
	}
}

void Landing::render(sf::RenderWindow& win, sf::Vector2f cam) {
	if (m_champion) { m_champion->render(win, cam); }
	// debug_window();
}

} // namespace fornani
