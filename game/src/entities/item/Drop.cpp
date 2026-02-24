
#include <fornani/entities/item/Drop.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::item {

Drop::Drop(automa::ServiceProvider& svc, world::Map& map, std::string_view key, float probability, int delay_time, int special_id)
	: Animatable(svc, key, {svc.data.drop[key]["sprite_dimensions"][0].as<int>(), svc.data.drop[key]["sprite_dimensions"][1].as<int>()}), sparkler(svc, drop_dimensions, colors::ui_white, "drop"), special_id(special_id), m_label{key},
	  m_collider{map, 16.f} {

	get_collider().physics.elasticity = 0.5f;

	m_sprite_dimensions.x = svc.data.drop[key]["sprite_dimensions"][0].as<float>();
	m_sprite_dimensions.y = svc.data.drop[key]["sprite_dimensions"][1].as<float>();
	center();

	type = static_cast<DropType>(svc.data.drop[key]["type"].as<int>());
	if (type == DropType::gem) { get_collider().physics.elasticity = 0.85f; }

	get_collider().physics.set_friction_componentwise({svc.data.drop[key]["friction"][0].as<float>(), svc.data.drop[key]["friction"][1].as<float>()});
	get_collider().physics.gravity = svc.data.drop[key]["gravity"].as<float>();
	get_collider().set_exclusion_target(shape::ColliderTrait::circle);
	get_collider().set_exclusion_target(shape::ColliderTrait::player);
	get_collider().set_exclusion_target(shape::ColliderTrait::npc);
	get_collider().set_exclusion_target(shape::ColliderTrait::enemy);

	auto& in_anim = svc.data.drop[key]["animation"];
	num_sprites = in_anim["num_sprites"].as<int>();
	for (auto param : in_anim["params"].as_array()) {
		anim::Parameters a{};
		a.lookup = param["lookup"].as<int>();
		a.duration = param["duration"].as<int>();
		a.framerate = param["framerate"].as<int>();
		a.num_loops = param["num_loops"].as<int>();
		m_parameters.push_back(a);
	}

	if (!m_parameters.empty()) { set_parameters(m_parameters[0]); }

	// randomly seed the animation start frame so drops in the same loot animate out of sync
	random_start();
	random_frame_start();

	auto rand_cooldown_offset = random::random_range(0, 50);
	auto const rand_shine_offset = random::random_range(0, 600);
	if (type == DropType::gem) { rand_cooldown_offset += 2000; }
	lifespan.start(4500 + rand_cooldown_offset);
	shine_cooldown.start(shine_cooldown.get_native_time() + rand_shine_offset);
	delay.start(delay_time);
	seed(probability);
	set_value();

	sparkler.set_dimensions(drop_dimensions);

	m_handle = 12000 + random::random_range(0, 1000);
}

void Drop::seed(float probability) {
	if (auto const random_sample = random::random_range_float(0.0f, 1.0f); random_sample < probability * constants.priceless) {
		rarity = Rarity::priceless;
	} else if (random_sample < probability * constants.rare) {
		rarity = Rarity::rare;
	} else if (random_sample < probability * constants.uncommon) {
		rarity = Rarity::uncommon;
	} else {
		rarity = Rarity::common;
	}
}

void Drop::set_value() {
	// heart
	if (type == DropType::heart) {
		if (rarity == Rarity::priceless || rarity == Rarity::rare) {
			value = 3;
		} else {
			value = 1;
		}
		return;
	}

	// orb
	switch (rarity) {
	case Rarity::priceless: value = 100; break;
	case Rarity::rare: value = 20; break;
	case Rarity::uncommon: value = 5; break;
	case Rarity::common: value = 1; break;
	}

	// gem
	if (type == DropType::gem) { value = special_id; }
}

void Drop::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (!check_delay(svc)) { return; }
	tick();
	delay.update();
	auto magnet = player.has_item_equipped("magnet");
	if (magnet) {
		get_collider().physics.set_friction_componentwise({0.995f, 0.995f});
		m_steering.seek(get_collider().physics, player.get_collider().get_center(), 0.0001f);
	} else {
		get_collider().physics.set_friction_componentwise({svc.data.drop[m_label]["friction"][0].as<float>(), svc.data.drop[m_label]["friction"][1].as<float>()});
	}
	get_collider().set_flag(shape::ColliderFlags::simple, magnet);
	for (auto& spike : map.spikes) { get_collider().handle_collision(spike.get_bounding_box()); }

	if (get_collider().collided()) {
		auto threshold = type == DropType::heart ? 0.1f : 1.f;
		if (!is_inactive() && std::abs(get_collider().physics.velocity.y) > threshold) {
			switch (type) {
			case DropType::gem: random::percent_chance(50) ? svc.soundboard.flags.world.set(audio::World::gem_hit_1) : svc.soundboard.flags.world.set(audio::World::gem_hit_2); break;
			case DropType::heart: svc.soundboard.play_sound("heart_collide", get_collider().get_global_center()); break;
			case DropType::orb: svc.soundboard.play_sound("orb_collide", get_collider().get_global_center()); break;
			}
		}
	}

	if (!magnet) { get_collider().physics.acceleration = {}; }
	lifespan.update();
	afterlife.update();

	sparkler.update(svc);
	sparkler.set_position(get_collider().position() - sparkler.get_dimensions() * 0.5f);

	int v{};
	if (type == DropType::heart) { v = rarity == Rarity::priceless || rarity == Rarity::rare ? 1 : 0; }
	if (type == DropType::orb) { v = rarity == Rarity::priceless ? 3 : (rarity == Rarity::rare ? 2 : (rarity == Rarity::uncommon ? 1 : 0)); }
	if (type == DropType::gem) { v = special_id; }
	set_channel(v);

	state_function = state_function();

	if (collides_with(player.get_collider().bounding_box) && !is_inactive() && !is_completely_gone() && delay_over()) {
		player.give_drop(get_type(), static_cast<float>(get_value()));
		if (get_type() == DropType::gem) {
			svc.soundboard.flags.item.set(audio::Item::gem);
		} else if (get_type() == DropType::heart) {
			svc.soundboard.flags.item.set(audio::Item::heal);
		} else if (get_rarity() == Rarity::common) {
			svc.soundboard.flags.item.set(audio::Item::orb_low);
		} else if (get_rarity() == Rarity::uncommon) {
			svc.soundboard.flags.item.set(audio::Item::orb_medium);
		} else if (get_rarity() == Rarity::rare) {
			svc.soundboard.flags.item.set(audio::Item::orb_high);
		} else if (get_rarity() == Rarity::priceless) {
			svc.soundboard.flags.item.set(audio::Item::orb_max);
		}
		deactivate();
	}
}

void Drop::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (m_start_delay) { return; }
	auto offset = sf::Vector2f{0.f, get_collider().get_radius() - Animatable::get_f_dimensions().y};
	Animatable::set_position(get_collider().get_global_center() + offset - cam);
	if (!is_inactive() && !is_completely_gone() && (lifespan.get() > 500 || (lifespan.get() / 20) % 2 == 0)) { win.draw(*this); }
	sparkler.render(win, cam);
	if (svc.greyblock_mode()) { get_collider().render(win, cam); }
}

void Drop::set_position(sf::Vector2f pos) {
	get_collider().physics.position = pos;
	sparkler.set_position(pos);
}

void Drop::apply_force(sf::Vector2f force, bool delayed) {
	if (delayed) {
		m_start_force = force;
	} else {
		get_collider().physics.apply_force(force);
	}
}

void Drop::set_delay(int time) {
	m_start_delay = util::Cooldown{time};
	m_start_delay->start();
}

void Drop::destroy_completely() {
	lifespan.cancel();
	afterlife.cancel();
}

void Drop::deactivate() {
	lifespan.cancel();
	afterlife.start(1000);
	sparkler.deactivate();
}

DropType Drop::get_type() const { return type; }

int Drop::get_value() const { return value; }

fsm::StateFunction Drop::update_neutral() {
	m_state.actual = DropState::neutral;
	shine_cooldown.update();
	if (Animatable::is_complete() && m_parameters.size() > 1) {
		request(DropState::shining);
		if (change_state(DropState::shining, m_parameters[1])) { return DROP_BIND(update_shining); }
	}
	return DROP_BIND(update_neutral);
}

fsm::StateFunction Drop::update_shining() {
	m_state.actual = DropState::shining;
	if (Animatable::is_complete()) {
		if (!m_parameters.empty()) {
			request(DropState::neutral);
			if (change_state(DropState::neutral, m_parameters[0])) { return DROP_BIND(update_neutral); }
		}
	}
	return DROP_BIND(update_shining);
}

bool Drop::change_state(DropState next, anim::Parameters params) {
	if (m_state.desired == next) {
		set_parameters(params);
		return true;
	}
	return false;
}

bool Drop::is_completely_gone() const { return afterlife.is_complete() && lifespan.is_complete(); }

bool Drop::is_inactive() const { return lifespan.is_complete() && !afterlife.is_complete(); }

bool Drop::check_delay(automa::ServiceProvider& svc) {
	if (m_start_delay) {
		get_collider().set_flag(shape::ColliderFlags::no_update);
		if (m_start_delay->running()) {
			m_start_delay->update();
		} else {
			get_collider().set_flag(shape::ColliderFlags::no_update, false);
			m_start_delay.reset();
			get_collider().physics.apply_force(m_start_force);
			svc.soundboard.flags.item.set(audio::Item::drop_spawn);
		}
		return false;
	}
	return true;
}

} // namespace fornani::item
