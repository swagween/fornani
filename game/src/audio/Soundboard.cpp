
#include <fornani/audio/Soundboard.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>

namespace fornani::audio {

constexpr auto minimum_wait_time_v = 16;

Soundboard::Soundboard(automa::ServiceProvider& svc, capo::IEngine& engine) : m_services(&svc), m_engine(&engine) {
	npc_map["bryn"] = make_int_setter<NPCBryn>(npc_flags.bryn);
	npc_map["gobe"] = make_int_setter<NPCGobe>(npc_flags.gobe);
	npc_map["minigus"] = make_int_setter<NPCMinigus>(npc_flags.minigus);
	npc_map["mirin"] = make_int_setter<NPCMirin>(npc_flags.mirin);

	auto filename = svc.finder.resource_path() + "/data/audio/sfx.json";
	auto sfx_data_result = dj::Json::from_file(filename);
	if (!sfx_data_result) {
		NANI_LOG_ERROR(m_logger, "Failed to load sfx data from path {}.", filename);
		return;
	}
	auto sfx_data = std::move(*sfx_data_result);
	for (auto const& entry : sfx_data.as_array()) { m_property_map.insert({entry["label"].as_string(), SoundProperties::from_json(entry)}); }
}

void Soundboard::play_sounds(capo::IEngine& engine, automa::ServiceProvider& svc, int echo_count, int echo_rate) {

	// update active sounds
	for (size_t i = 0; i < sound_pool.size();) {
		auto& s = sound_pool[i];
		s.sound.update(svc, s.position - m_listener.position);

		if (s.looping && !s.touched_this_tick) { s.sound.fade_out(); }
		s.touched_this_tick = false;

		if (!s.sound.is_running()) {
			std::swap(s, sound_pool.back());
			sound_pool.pop_back();
		} else {
			++i;
		}
	}

	// menu
	if (flags.menu.test(Menu::forward_switch)) { play(engine, svc, "menu_next"); }
	if (flags.menu.test(Menu::backward_switch)) { play(engine, svc, "menu_back"); }
	if (flags.menu.test(Menu::select)) { play(engine, svc, "menu_click"); }
	if (flags.menu.test(Menu::shift)) { play(engine, svc, "menu_shift"); }
	if (flags.menu.test(Menu::error)) { play(engine, svc, "error"); }

	// console
	if (flags.console.test(Console::select)) { play(engine, svc, "menu_click"); }
	if (flags.console.test(Console::done)) { play(engine, svc, "menu_back"); }
	if (flags.console.test(Console::next)) { play(engine, svc, "menu_next"); }
	if (flags.console.test(Console::shift)) { play(engine, svc, "menu_shift"); }
	if (flags.console.test(Console::menu_open)) { play(engine, svc, "menu_open"); }
	if (flags.console.test(Console::notification)) { play(engine, svc, "notification"); }

	// pioneer
	if (flags.pioneer.test(Pioneer::back)) { play(engine, svc, "pioneer_back"); }
	if (flags.pioneer.test(Pioneer::click)) { play(engine, svc, "pioneer_click"); }
	if (flags.pioneer.test(Pioneer::select)) { play(engine, svc, "pioneer_select"); }
	if (flags.pioneer.test(Pioneer::open)) { play(engine, svc, "pioneer_open"); }
	if (flags.pioneer.test(Pioneer::close)) { play(engine, svc, "pioneer_close"); }
	if (flags.pioneer.test(Pioneer::slot)) { play(engine, svc, "pioneer_slot"); }
	if (flags.pioneer.test(Pioneer::chain)) { play(engine, svc, "pioneer_chain"); }
	if (flags.pioneer.test(Pioneer::drag)) { play(engine, svc, "pioneer_drag"); }
	if (flags.pioneer.test(Pioneer::wires)) { play(engine, svc, "pioneer_wires"); }
	if (flags.pioneer.test(Pioneer::forward)) { play(engine, svc, "pioneer_forward"); }
	if (flags.pioneer.test(Pioneer::unhover)) { play(engine, svc, "pioneer_unhover"); }
	if (flags.pioneer.test(Pioneer::hard_slot)) { play(engine, svc, "pioneer_hard_slot"); }
	if (flags.pioneer.test(Pioneer::fast_click)) { play(engine, svc, "pioneer_fast_click"); }
	if (flags.pioneer.test(Pioneer::sync)) { play(engine, svc, "pioneer_sync", 0.1f); }
	if (flags.pioneer.test(Pioneer::boot)) { play(engine, svc, "pioneer_boot", 0.f, 40.f); }

	// transmission
	if (flags.transmission.test(Transmission::statics)) { play(engine, svc, "radio_begin_transmission"); }

	// always play console and menu sounds
	if (status == SoundboardState::off) {
		flags = {};
		proximities = {};
		return;
	}

	// world
	if (flags.world.test(World::load)) { play(engine, svc, "load_game", 0.f, 40.f); }
	if (flags.world.test(World::save)) { play(engine, svc, "save_game"); }
	if (flags.world.test(World::chest)) { play(engine, svc, "chest_open", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.world.test(World::breakable_shatter)) { play(engine, svc, "breakable_shatter", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.world.test(World::breakable_hit)) { play(engine, svc, "breakable_hit", 0.1f); }
	if (flags.world.test(World::hard_hit)) { play(engine, svc, "hard_hit", 0.1f, 60.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.world.test(World::projectile_hit)) { play(engine, svc, "hit", 0.2f); }
	if (flags.world.test(World::clink)) { play(engine, svc, "clink", 0.2f); }
	if (flags.world.test(World::vibration)) { play(engine, svc, "vibration"); }
	if (flags.world.test(World::big_crash)) { play(engine, svc, "big_crash"); }
	if (flags.world.test(World::heavy_land)) { play(engine, svc, "heavy_land"); }
	if (flags.world.test(World::delay_crash)) { play(engine, svc, "delay_crash", 0.f, 50.f); }
	// flags.world.test(World::laser_hum) ? simple_repeat(engine, svc.sounds.get_buffer("laser_hum"), "laser_hum") : stop("laser_hum");
	if (flags.world.test(World::incinerite_explosion)) { play(engine, svc, "incinerite_explosion", 0.2f); }
	if (flags.world.test(World::splash)) { play(engine, svc, "splash", 0.3f, 60.f); }

	// vendor
	if (flags.item.test(Item::vendor_sale)) { play(engine, svc, "vendor_sale"); }

	if (!svc.in_game()) {
		flags = {};
		return;
	} // exit early if not in-game

	if (flags.world.test(World::wall_hit)) { play(engine, svc, "wall_hit", 0.1f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.world.test(World::thud)) { play(engine, svc, "thud", 0.1f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.world.test(World::switch_press)) { play(engine, svc, "switch_press", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.world.test(World::block_toggle)) { play(engine, svc, "block_toggle", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.world.test(World::door_open)) { play(engine, svc, "door_open", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.world.test(World::door_unlock)) { play(engine, svc, "door_unlock"); }
	if (flags.world.test(World::gem_hit_1)) { play(engine, svc, "gem_hit_1", 0.2f, 50.f); }
	if (flags.world.test(World::gem_hit_2)) { play(engine, svc, "gem_hit_2", 0.2f, 50.f); }

	// hulmet
	if (flags.hulmet.test(Hulmet::hurt)) { play(engine, svc, "hulmet_hurt", 0.1f); }
	if (flags.hulmet.test(Hulmet::alert)) { play(engine, svc, "hulmet_alert", 0.1f); }
	if (flags.hulmet.test(Hulmet::reload)) { play(engine, svc, "hulmet_reload"); }

	// tank
	// if (flags.tank.test(Tank::alert_1)) { play(engine, svc, "tank_alert_1"); }
	// if (flags.tank.test(Tank::alert_2)) { play(engine, svc, "tank_alert_2"); }
	if (flags.tank.test(Tank::hurt_1)) { play(engine, svc, "tank_hurt_1"); }
	if (flags.tank.test(Tank::hurt_2)) { play(engine, svc, "tank_hurt_2", 0.f, 50.f); }
	// if (flags.tank.test(Tank::death)) { play(engine, svc, "tank_death"); }
	// if (flags.tank.test(Tank::step)) { play(engine, svc, "tank_step"); }
	// if (flags.tank.test(Tank::sip)) { play(engine, svc, "tank_sip"); }

	// mastiff
	if (flags.mastiff.test(Mastiff::bite)) { play(engine, svc, "mastiff_bite", 0.f, 50.f); }
	if (flags.mastiff.test(Mastiff::growl)) { play(engine, svc, "mastiff_growl", 0.2f, 50.f); }

	// meatsquash
	if (flags.meatsquash.test(Meatsquash::chomp)) { play(engine, svc, "beast_charge_up"); }
	if (flags.meatsquash.test(Meatsquash::whip)) { play(engine, svc, "beast_whip"); }
	if (flags.meatsquash.test(Meatsquash::swallow)) { play(engine, svc, "beast_swallow"); }
	if (flags.meatsquash.test(Meatsquash::open)) { play(engine, svc, "beast_chew"); }
	if (flags.meatsquash.test(Meatsquash::death)) { play(engine, svc, "beast_death"); }

	// minigus
	if (flags.minigus.test(Minigus::invincible)) { play(engine, svc, "minigus_invincible"); }
	if (flags.minigus.test(Minigus::build)) { play(engine, svc, "minigus_build"); }
	/*if (flags.minigus.test(Minigus::hurt_1)) { play(engine, svc, "minigus_hurt_1"); }
	if (flags.minigus.test(Minigus::hurt_2)) { play(engine, svc, "minigus_hurt_2"); }
	if (flags.minigus.test(Minigus::hurt_3)) { play(engine, svc, "minigus_hurt_3"); }*/
	if (flags.minigus.test(Minigus::laugh_1)) { play(engine, svc, "minigus_laugh_1"); }
	if (flags.minigus.test(Minigus::laugh_2)) { play(engine, svc, "minigus_laugh_2"); }
	if (flags.minigus.test(Minigus::snap)) { play(engine, svc, "minigus_snap"); }
	if (flags.minigus.test(Minigus::crash)) { play(engine, svc, "minigus_crash"); }
	if (flags.minigus.test(Minigus::land)) { play(engine, svc, "minigus_land"); }
	if (flags.minigus.test(Minigus::woob)) { play(engine, svc, "minigus_woob"); }
	if (flags.minigus.test(Minigus::getit)) { play(engine, svc, "minigus_getit"); }
	if (flags.minigus.test(Minigus::mother)) { play(engine, svc, "minigus_mother"); }
	if (flags.minigus.test(Minigus::momma)) { play(engine, svc, "minigus_momma"); }
	if (flags.minigus.test(Minigus::deepspeak)) { play(engine, svc, "minigus_deepspeak"); }
	if (flags.minigus.test(Minigus::quick_breath)) { play(engine, svc, "minigus_quick_breath"); }
	if (flags.minigus.test(Minigus::long_moan)) { play(engine, svc, "minigus_long_moan"); }
	if (flags.minigus.test(Minigus::poh)) { play(engine, svc, "minigus_poh"); }
	if (flags.minigus.test(Minigus::soda)) { play(engine, svc, "minigus_soda"); }
	if (flags.minigus.test(Minigus::doge)) { play(engine, svc, "minigus_doge"); }
	if (flags.minigus.test(Minigus::charge)) { play(engine, svc, "minigus_gun_charge"); }
	if (flags.minigus.test(Minigus::pizza)) { play(engine, svc, "minigus_pizza"); }
	if (flags.minigus.test(Minigus::greatidea)) { play(engine, svc, "minigus_greatidea"); }
	if (flags.minigus.test(Minigus::dontlookatme)) { play(engine, svc, "minigus_dontlookatme"); }
	if (flags.minigus.test(Minigus::grunt)) { play(engine, svc, "minigus_grunt"); }
	if (flags.minigus.test(Minigus::ok)) { play(engine, svc, "minigus_ok_1"); }
	if (flags.minigus.test(Minigus::exhale)) { play(engine, svc, "minigus_exhale"); }

	// lynx
	if (flags.lynx.test(Lynx::ping_1)) { play(engine, svc, "lynx_ping_1", 0.1f); }
	if (flags.lynx.test(Lynx::ping_2)) { play(engine, svc, "lynx_ping_2", 0.1f); }
	if (flags.lynx.test(Lynx::swipe_1)) { play(engine, svc, "lynx_swipe_1", 0.1f); }
	if (flags.lynx.test(Lynx::swipe_2)) { play(engine, svc, "lynx_swipe_2", 0.1f); }
	if (flags.lynx.test(Lynx::shing)) { play(engine, svc, "lynx_shing", 0.f, 70.f); }
	if (flags.lynx.test(Lynx::prepare)) { play(engine, svc, "lynx_prepare"); }
	if (flags.lynx.test(Lynx::slam)) { play(engine, svc, "lynx_slam", 0.f, 70.f); }
	if (flags.lynx.test(Lynx::hoah)) { play(engine, svc, "lynx_hoah", 0.f, 70.f); }
	if (flags.lynx.test(Lynx::defeat)) { play(engine, svc, "lynx_defeat", 0.f, 70.f); }
	if (flags.lynx.test(Lynx::hah)) { play(engine, svc, "lynx_hah", 0.f, 70.f); }
	if (flags.lynx.test(Lynx::heuh)) { play(engine, svc, "lynx_heuh", 0.f, 70.f); }
	if (flags.lynx.test(Lynx::hiyyah)) { play(engine, svc, "lynx_hiyyah", 0.f, 70.f); }
	if (flags.lynx.test(Lynx::hnnyah)) { play(engine, svc, "lynx_hnnyah", 0.f, 70.f); }
	if (flags.lynx.test(Lynx::huh)) { play(engine, svc, "lynx_huh", 0.f, 70.f); }
	if (flags.lynx.test(Lynx::hurt_1)) { play(engine, svc, "lynx_hurt_1", 0.f); }
	if (flags.lynx.test(Lynx::hurt_2)) { play(engine, svc, "lynx_hurt_2", 0.f, 70.f); }
	if (flags.lynx.test(Lynx::hurt_3)) { play(engine, svc, "lynx_hurt_3", 0.f, 40.f); }
	if (flags.lynx.test(Lynx::hurt_4)) { play(engine, svc, "lynx_hurt_4", 0.f, 40.f); }
	if (flags.lynx.test(Lynx::huuyeah)) { play(engine, svc, "lynx_huuyeah", 0.f, 70.f); }
	if (flags.lynx.test(Lynx::nngyah)) { play(engine, svc, "lynx_nngyah", 0.f, 70.f); }
	if (flags.lynx.test(Lynx::yyah)) { play(engine, svc, "lynx_yyah", 0.f, 70.f); }
	if (flags.lynx.test(Lynx::laugh)) { play(engine, svc, "lynx_laugh", 0.f, 70.f); }
	if (flags.lynx.test(Lynx::giggle)) { play(engine, svc, "lynx_giggle", 0.f, 70.f); }

	// miaag
	if (flags.miaag.test(Miaag::growl)) { play(engine, svc, "miaag_growl"); }
	if (flags.miaag.test(Miaag::hiss)) { play(engine, svc, "miaag_hiss"); }
	if (flags.miaag.test(Miaag::hurt)) { play(engine, svc, "miaag_hurt", 0.1f, 50.f); }
	if (flags.miaag.test(Miaag::roar)) { play(engine, svc, "miaag_roar", 0.1f, 50.f); }
	if (flags.miaag.test(Miaag::chomp)) { play(engine, svc, "miaag_chomp"); }

	// npc
	if (npc_flags.minigus.test(NPCMinigus::getit)) { play(engine, svc, "minigus_getit"); }
	if (npc_flags.minigus.test(NPCMinigus::laugh)) { play(engine, svc, "minigus_laugh_1"); }
	if (npc_flags.minigus.test(NPCMinigus::pizza)) { play(engine, svc, "minigus_pizza"); }
	if (npc_flags.minigus.test(NPCMinigus::greatidea)) { play(engine, svc, "minigus_greatidea"); }
	if (npc_flags.minigus.test(NPCMinigus::dontlookatme)) { play(engine, svc, "minigus_dontlookatme"); }
	if (npc_flags.minigus.test(NPCMinigus::grunt)) { play(engine, svc, "minigus_grunt"); }

	// bryn
	auto bryn_volume = 60.f;
	if (npc_flags.bryn.test(NPCBryn::agh)) { play(engine, svc, "bryn_agh", 0.f, 30.f); }
	if (npc_flags.bryn.test(NPCBryn::ah_1)) { play(engine, svc, "bryn_ah_1", 0.f, bryn_volume); }
	if (npc_flags.bryn.test(NPCBryn::ah_2)) { play(engine, svc, "bryn_ah_2", 0.f, bryn_volume); }
	if (npc_flags.bryn.test(NPCBryn::chuckle)) { play(engine, svc, "bryn_chuckle", 0.f, bryn_volume); }
	if (npc_flags.bryn.test(NPCBryn::nani_1)) { play(engine, svc, "bryn_nani_1", 0.f, bryn_volume); }
	if (npc_flags.bryn.test(NPCBryn::nani_2)) { play(engine, svc, "bryn_nani_2", 0.f, bryn_volume); }
	if (npc_flags.bryn.test(NPCBryn::oh)) { play(engine, svc, "bryn_oh", 0.f, bryn_volume); }
	if (npc_flags.bryn.test(NPCBryn::ohh)) { play(engine, svc, "bryn_ohh", 0.f, bryn_volume); }
	if (npc_flags.bryn.test(NPCBryn::sigh)) { play(engine, svc, "bryn_sigh", 0.f); }
	if (npc_flags.bryn.test(NPCBryn::whatsup)) { play(engine, svc, "bryn_whatsup", 0.f, bryn_volume); }
	if (npc_flags.bryn.test(NPCBryn::yah)) { play(engine, svc, "bryn_yah", 0.f, bryn_volume); }
	if (npc_flags.bryn.test(NPCBryn::yeah)) { play(engine, svc, "bryn_yeah", 0.f, bryn_volume); }
	if (npc_flags.bryn.test(NPCBryn::yeahh)) { play(engine, svc, "bryn_yeahh", 0.f, bryn_volume); }
	if (npc_flags.bryn.test(NPCBryn::eagh)) { play(engine, svc, "bryn_eagh", 0.f, bryn_volume); }
	if (npc_flags.bryn.test(NPCBryn::haha)) { play(engine, svc, "bryn_haha", 0.f, bryn_volume); }
	if (npc_flags.bryn.test(NPCBryn::hello)) { play(engine, svc, "bryn_hello", 0.f, bryn_volume); }
	if (npc_flags.bryn.test(NPCBryn::hey_1)) { play(engine, svc, "bryn_hey_1", 0.f, bryn_volume); }
	if (npc_flags.bryn.test(NPCBryn::hey_2)) { play(engine, svc, "bryn_hey_2", 0.f, 80.f); }
	if (npc_flags.bryn.test(NPCBryn::heyyy)) { play(engine, svc, "bryn_heyyy", 0.f, bryn_volume); }
	if (npc_flags.bryn.test(NPCBryn::hi)) { play(engine, svc, "bryn_hi", 0.f, bryn_volume); }
	if (npc_flags.bryn.test(NPCBryn::hmm)) { play(engine, svc, "bryn_hmm", 0.f, bryn_volume); }
	if (npc_flags.bryn.test(NPCBryn::hmph)) { play(engine, svc, "bryn_hmph", 0.f, bryn_volume); }
	if (npc_flags.bryn.test(NPCBryn::laugh_1)) { play(engine, svc, "bryn_laugh_1", 0.f, bryn_volume); }
	if (npc_flags.bryn.test(NPCBryn::laugh_2)) { play(engine, svc, "bryn_laugh_2", 0.f, bryn_volume); }
	if (npc_flags.bryn.test(NPCBryn::mm)) { play(engine, svc, "bryn_mm", 0.f, bryn_volume); }
	if (npc_flags.bryn.test(NPCBryn::oeugh)) { play(engine, svc, "bryn_oeugh", 0.f, bryn_volume); }

	// gobe
	if (npc_flags.gobe.test(NPCGobe::oh)) { play(engine, svc, "gobe_oh"); }
	if (npc_flags.gobe.test(NPCGobe::orewa)) { play(engine, svc, "gobe_orewa"); }

	// mirin
	if (npc_flags.mirin.test(NPCMirin::ah)) { play(engine, svc, "mirin_ah"); }
	if (npc_flags.mirin.test(NPCMirin::haha)) { play(engine, svc, "mirin_laugh"); }
	if (npc_flags.mirin.test(NPCMirin::oh)) { play(engine, svc, "mirin_oh"); }

	// thug
	if (flags.thug.test(Thug::alert_1)) { play(engine, svc, "tank_alert_1"); }
	if (flags.thug.test(Thug::alert_2)) { play(engine, svc, "tank_alert_2"); }
	if (flags.thug.test(Thug::hurt_1)) { play(engine, svc, "tank_hurt_1"); }
	if (flags.thug.test(Thug::hurt_2)) { play(engine, svc, "tank_hurt_2", 0.f, 50.f); }
	if (flags.thug.test(Thug::death)) { play(engine, svc, "tank_death"); }

	// demon
	if (flags.demon.test(Demon::hurt)) { play(engine, svc, "demon_hurt"); }
	if (flags.demon.test(Demon::death)) { play(engine, svc, "demon_death"); }
	if (flags.demon.test(Demon::snort)) { play(engine, svc, "demon_snort", 0.2f); }
	if (flags.demon.test(Demon::up_snort)) { play(engine, svc, "demon_up_snort", 0.2f); }
	if (flags.demon.test(Demon::alert)) { play(engine, svc, "demon_alert"); }

	// summoner
	if (flags.summoner.test(Summoner::block_1)) { play(engine, svc, "summoner_block_1", 0.2f, 20.f); }
	if (flags.summoner.test(Summoner::block_2)) { play(engine, svc, "summoner_block_2", 0.2f, 20.f); }
	if (flags.summoner.test(Summoner::hurt_1)) { play(engine, svc, "summoner_hurt_1", 0.2f, 20.f); }
	if (flags.summoner.test(Summoner::hurt_2)) { play(engine, svc, "summoner_hurt_2", 0.2f, 20.f); }
	if (flags.summoner.test(Summoner::summon)) { play(engine, svc, "summoner_summon", 0.2f, 40.f); }

	// general enemy
	if (flags.enemy.test(Enemy::hit_low)) { play(engine, svc, "hit_low"); }
	if (flags.enemy.test(Enemy::hit_medium)) { play(engine, svc, "hit_medium"); }
	if (flags.enemy.test(Enemy::hit_high)) { play(engine, svc, "hit_high"); }
	if (flags.enemy.test(Enemy::hit_squeak)) { play(engine, svc, "hit_squeak"); }
	if (flags.enemy.test(Enemy::standard_death)) { play(engine, svc, "standard_death"); }
	if (flags.enemy.test(Enemy::high_death)) { play(engine, svc, "high_death"); }
	if (flags.enemy.test(Enemy::low_death)) { play(engine, svc, "low_death"); }
	if (flags.enemy.test(Enemy::jump_low)) { play(engine, svc, "jump_low"); }
	if (flags.enemy.test(Enemy::disappear)) { play(engine, svc, "disappear"); }

	// general beast
	if (flags.beast.test(Beast::growl)) { play(engine, svc, "beast_growl", 0.f, 70.f); }
	if (flags.beast.test(Beast::hurt)) { play(engine, svc, "beast_hurt", 0.2f, 50.f); }

	// beamsprout
	if (flags.beamsprout.test(Beamsprout::hurt)) { play(engine, svc, "hit_high", 0.2f, 50.f); }
	if (flags.beamsprout.test(Beamsprout::charge)) { play(engine, svc, "small_beast_charge", 0.f, 40.f); }
	if (flags.beamsprout.test(Beamsprout::shoot)) { play(engine, svc, "beast_spit", 0.2f, 80.f); }

	// crow
	if (flags.crow.test(Crow::fly)) { play(engine, svc, "crow_fly", 0.2f, 20.f); }
	if (flags.crow.test(Crow::flap)) { play(engine, svc, "crow_flap", 0.2f, 20.f); }

	// item
	if (flags.item.test(Item::heal)) { play(engine, svc, "heal", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.item.test(Item::orb_low)) { play(engine, svc, "orb_get_1", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.item.test(Item::orb_medium)) { play(engine, svc, "orb_get_2", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.item.test(Item::orb_high)) { play(engine, svc, "orb_get_3", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.item.test(Item::orb_max)) { play(engine, svc, "orb_get_4", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.item.test(Item::health_increase)) { play(engine, svc, "health_increase"); }
	if (flags.item.test(Item::gem)) { play(engine, svc, "gem_get", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.item.test(Item::get)) { play(engine, svc, "item_get"); }
	if (flags.item.test(Item::equip)) { play(engine, svc, "item_equip"); }
	if (flags.item.test(Item::unequip)) { play(engine, svc, "item_unequip"); }
	if (flags.item.test(Item::drop_spawn)) { play(engine, svc, "drop_spawn", 0.2f); }
	// if (flags.item.test(Item::orb_collide)) { play(engine, svc, "orb_collide", 0.2f); }
	// if (flags.item.test(Item::heart_collide)) { play(engine, svc, "heart_collide", 0.2f); }

	// player
	if (flags.player.test(Player::jump)) { play(engine, svc, "nani_jump", 0.1f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.player.test(Player::hurt)) { play(engine, svc, "nani_hurt", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.player.test(Player::gulp)) { play(engine, svc, "nani_gulp", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.player.test(Player::dash)) { play(engine, svc, "nani_dash", 0.1f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.player.test(Player::death)) { play(engine, svc, "nani_death"); }
	if (flags.player.test(Player::slide)) { play(engine, svc, "nani_slide", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.player.test(Player::super_slide)) { play(engine, svc, "nani_super_slide", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.player.test(Player::walljump)) { play(engine, svc, "nani_walljump", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.player.test(Player::doublejump)) { play(engine, svc, "nani_doublejump", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.player.test(Player::roll)) { play(engine, svc, "nani_roll", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	/*flags.player.test(Player::wallslide) ? simple_repeat(engine, svc.sounds.get_buffer("nani_wallslide"), "nani_wallslide", 32) : fade_out("nani_wallslide");
	flags.player.test(Player::turn_slide) ? simple_repeat(engine, svc.sounds.get_buffer("nani_turn_slide"), "nani_turn_slide", 8) : fade_out("nani_turn_slide");*/
	if (flags.player.test(Player::dash_kick)) { play(engine, svc, "nani_dash_kick", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.player.test(Player::dive)) { play(engine, svc, "nani_dive", 0.3f, 50.f, 0, 1.f, {}, echo_count, echo_rate); }

	// steps
	if (flags.step.test(Step::basic)) { play(engine, svc, "nani_steps", 0.1f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.step.test(Step::grass)) { play(engine, svc, "nani_steps_grass", 0.3f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.land.test(Step::basic)) { play(engine, svc, "nani_landed", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.land.test(Step::grass)) { play(engine, svc, "nani_landed_grass", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }

	// arms
	if (flags.player.test(Player::arms_switch)) { play(engine, svc, "arms_switch", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.arms.test(Arms::reload)) { play(engine, svc, "arms_reload", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.arms.test(Arms::frag_grenade)) { play(engine, svc, "frag_grenade", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.arms.test(Arms::whistle)) { play(engine, svc, "missile_whistle", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }

	/*if (flags.weapon.test(Weapon::bryns_gun)) { play(engine, svc, "arms_shot_bg", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.weapon.test(Weapon::gnat)) { play(engine, svc, "arms_shot_gnat", 0.1f, 100.f, 2, 1.f, {}, echo_count, echo_rate); }
	if (flags.weapon.test(Weapon::wasp)) { play(engine, svc, "arms_shot_wasp", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.weapon.test(Weapon::tomahawk)) { play(engine, svc, "arms_shot_tomahawk", 0.1f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.weapon.test(Weapon::skycorps_ar)) { play(engine, svc, "arms_shot_skycorps_ar", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.weapon.test(Weapon::demon_magic)) { play(engine, svc, "arms_shot_demon_magic", 0.1f, 40.f); }
	if (flags.weapon.test(Weapon::nova)) { play(engine, svc, "arms_shot_nova", 0.1f); }*/

	if (flags.weapon.test(Weapon::tomahawk_catch)) { play(engine, svc, "arms_catch_tomahawk", 0.1f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.weapon.test(Weapon::pulse)) { play(engine, svc, "magic_pulse", 0.2f, 20.f); }

	if (flags.weapon.test(Weapon::energy_ball)) { play(engine, svc, "arms_shot_energy_ball", 0.1f); }

	if (flags.projectile.test(Projectile::basic)) { play(engine, svc, "wall_hit", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.projectile.test(Projectile::shuriken)) { play(engine, svc, "clink", 0.2f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.projectile.test(Projectile::pulse)) { play(engine, svc, "projectile_pulse", 0.2f, 10.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.projectile.test(Projectile::hard_hit)) { play(engine, svc, "hard_hit", 0.2f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.projectile.test(Projectile::critical_hit)) { play(engine, svc, "critical_hit", 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }

	// reset flags
	flags = {};
	npc_flags = {};
}

void Soundboard::play_sound(std::string_view label) { play_sound(label, m_listener.position); }

void Soundboard::play_sound(std::string_view label, sf::Vector2f position) {
	auto it = m_property_map.find(label);
	if (it == m_property_map.end()) { it = m_property_map.find("error_sound"); }

	std::string_view lookup = label;

	auto tick = m_services->ticker.ticks;
	if (tick - minimum_wait_time_v < m_services->sounds.get_tick_for_buffer(lookup)) { return; }
	m_services->sounds.set_tick_for_buffer(lookup, tick);

	auto props = it->second;
	props.volume *= m_volume_multiplier;

	auto& entry = sound_pool.emplace_back(ActiveSound{.sound = Sound(*m_engine, m_services->sounds.get_buffer(lookup), lookup, props), .label = lookup, .id = 0, .position = position, .looping = false, .touched_this_tick = true});

	entry.sound.play();
}

void Soundboard::repeat_sound(std::string_view label, SoundProducerID id, sf::Vector2f position, float pitch) {
	auto it = m_property_map.find(label);
	auto not_found = it == m_property_map.end();
	if (not_found) { it = m_property_map.find("error_sound"); }

	std::string_view lookup = not_found ? label : it->first;
	auto props = it->second;
	props.volume *= m_volume_multiplier;

	auto existing = std::find_if(sound_pool.begin(), sound_pool.end(), [&](ActiveSound& s) { return s.looping && s.label == lookup && s.id == id; });

	if (id == 0) { position = m_listener.position; }

	if (existing != sound_pool.end()) {
		existing->position = position;
		existing->sound.set_pitch(pitch);
		existing->touched_this_tick = true;
		return;
	}

	auto& entry = sound_pool.emplace_back(ActiveSound{.sound = Sound(*m_engine, m_services->sounds.get_buffer(lookup), lookup, props), .label = lookup, .id = id, .position = position, .looping = true, .touched_this_tick = true});

	entry.sound.play(true);
}

void Soundboard::play(capo::IEngine& engine, automa::ServiceProvider& svc, std::string const& label, float random_pitch_offset, float vol, int frequency, float attenuation, sf::Vector2f distance, int echo_count, int echo_rate) {
	if (vol == 0.f) return;
	auto it = m_property_map.find(label);
	if (it == m_property_map.end()) { it = m_property_map.find("error_sound"); }

	std::string_view lookup = it->first;
	auto props = it->second;

	auto tick = svc.ticker.ticks;
	if (frequency == 0 && tick - minimum_wait_time_v < svc.sounds.get_tick_for_buffer(label)) { return; }

	props.volume *= (vol / 100.f) * m_volume_multiplier;

	svc.sounds.set_tick_for_buffer(label, tick);
	auto& entry = sound_pool.emplace_back(ActiveSound{.sound = Sound(engine, svc.sounds.get_buffer(label), lookup, props), .label = label, .position = m_listener.position, .looping = frequency != 0, .touched_this_tick = true});
	if (frequency != 0) {
		repeat(svc, entry.sound, frequency, random_pitch_offset, attenuation, distance);
	} else {
		randomize(svc, entry.sound, random_pitch_offset, vol, attenuation, distance);
	}
}

void Soundboard::play(capo::IEngine& engine, automa::ServiceProvider& svc, std::string const& label, SoundProperties properties, int frequency, float attenuation) {
	auto it = m_property_map.find(label);
	if (it == m_property_map.end()) it = m_property_map.find("error_sound");

	std::string_view lookup = it->first;
	auto props = it->second;

	props.volume *= m_volume_multiplier;
	if (props.volume < constants::tiny_value) { return; }

	auto tick = svc.ticker.ticks;
	if (frequency == 0 && tick - minimum_wait_time_v < svc.sounds.get_tick_for_buffer(lookup)) { return; }
	svc.sounds.set_tick_for_buffer(lookup, tick);
	auto& entry = sound_pool.emplace_back(ActiveSound{.sound = Sound(engine, svc.sounds.get_buffer(label), lookup, props), .label = lookup, .position = m_listener.position, .looping = frequency != 0, .touched_this_tick = true});
	if (frequency != 0) {
		repeat(svc, entry.sound, frequency, props.pitch_offset, attenuation, {});
	} else {
		entry.sound.play();
	}
}

void Soundboard::stop(std::string_view label) {
	std::erase_if(sound_pool, [label](auto const& s) { return s.label == label; });
}

void Soundboard::repeat(automa::ServiceProvider& svc, Sound& sound, int frequency, float random_pitch_offset, float attenuation, sf::Vector2f distance) {
	if (frequency == -1) {
		randomize(svc, sound, random_pitch_offset, 100.f, attenuation, distance, true);
	} else if (svc.ticker.every_x_ticks(frequency)) {
		randomize(svc, sound, random_pitch_offset, 100.f, attenuation, distance);
	}
}

void Soundboard::randomize(automa::ServiceProvider& svc, Sound& sound, float random_pitch_offset, float vol, float attenuation, sf::Vector2f distance, bool wait_until_over) {
	auto random_pitch = random_pitch_offset == 0.f ? 0.f : random::random_range_float(-random_pitch_offset, random_pitch_offset);
	sound.set_pitch(1.f + random_pitch);
	auto scalar = distance.length() / attenuation;
	sound.set_volume((vol - (scalar > vol ? vol : scalar)));
	if (wait_until_over && sound.is_playing()) { return; }
	sound.play();
}

void Soundboard::play_step(int tile_value, int style_id, bool land) {
	auto& set = land ? flags.land : flags.step;
	if (!get_step_sound.contains(style_id)) {
		set.set(audio::Step::basic);
		return;
	}
	if (!get_step_sound.at(style_id).contains(tile_value)) {
		set.set(audio::Step::basic);
		return;
	}
	set.set(get_step_sound.at(style_id).at(tile_value));
}

auto Soundboard::number_of_playng_sounds() -> int { return static_cast<int>(sound_pool.size()); }

} // namespace fornani::audio
