
#include "fornani/weapon/Weapon.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::arms {

constexpr auto minimum_speed_multiplier_v = 0.2f;
constexpr auto minimum_damage_multiplier_v = 0.5f;

Weapon::Weapon(automa::ServiceProvider& svc, std::string_view tag, bool enemy)
	: Animatable(svc, "guns", {32, 8}), metadata{.id = enemy ? svc.data.enemy_weapon[tag]["metadata"]["id"].as<int>() : svc.data.weapon[tag]["metadata"]["id"].as<int>(),
												 .tag = tag.data(),
												 .label = enemy ? svc.data.enemy_weapon[tag]["metadata"]["label"].as_string() : svc.data.weapon[tag]["metadata"]["label"].as_string()},
	  projectile(svc, tag, enemy ? svc.data.enemy_weapon[tag]["metadata"]["id"].as<int>() : svc.data.weapon[tag]["metadata"]["id"].as<int>(), *this, enemy), visual{.ui{sf::Sprite{svc.assets.get_texture("inventory_guns")}}},
	  cooldowns{.shoot_effect{64}}, m_services{&svc} {

	auto const& in_data = enemy ? svc.data.enemy_weapon[tag] : svc.data.weapon[tag];

	// metadata
	metadata.description = in_data["metadata"]["description"].as_string();

	// visual
	visual.dimensions = {in_data["visual"]["dimensions"][0].as<int>(), in_data["visual"]["dimensions"][1].as<int>()};
	offsets.render.global = {in_data["visual"]["offsets"]["global"][0].as<float>(), in_data["visual"]["offsets"]["global"][1].as<float>()};
	offsets.render.barrel = {in_data["visual"]["offsets"]["barrel"][0].as<float>(), in_data["visual"]["offsets"]["barrel"][1].as<float>()};
	emitter.dimensions = {in_data["visual"]["spray"]["dimensions"][0].as<float>(), in_data["visual"]["spray"]["dimensions"][1].as<float>()};
	emitter.type = in_data["visual"]["spray"]["type"].as_string().data(); // secondary emitter

	if (in_data["visual"]["secondary_spray"]) {
		secondary_emitter = EmitterAttributes();
		secondary_emitter.value().dimensions.x = in_data["visual"]["secondary_spray"]["dimensions"][0].as<float>();
		secondary_emitter.value().dimensions.y = in_data["visual"]["secondary_spray"]["dimensions"][1].as<float>();
		secondary_emitter.value().type = in_data["visual"]["secondary_spray"]["type"].as_string();
	}
	set_origin(offsets.render.global);

	// gameplay
	ammo.set_max(in_data["gameplay"]["attributes"]["ammo"].as<int>());
	cooldowns.reload = util::Cooldown{in_data["gameplay"]["attributes"]["reload"].as<int>()};
	specifications.multishot = in_data["gameplay"]["attributes"]["multishot"].as<int>();
	specifications.cooldown_time = in_data["gameplay"]["attributes"]["cooldown_time"].as<int>();
	specifications.reload_time = in_data["gameplay"]["attributes"]["reload"].as<int>();
	specifications.recoil = in_data["gameplay"]["attributes"]["recoil"].as<float>();
	if (static_cast<bool>(in_data["gameplay"]["attributes"]["automatic"].as_bool())) { attributes.set(WeaponAttributes::automatic); }
	if (static_cast<bool>(in_data["gameplay"]["attributes"]["no_reload"].as_bool())) { attributes.set(WeaponAttributes::no_reload); }
	if (static_cast<bool>(in_data["gameplay"]["attributes"]["charge"].as_bool())) {
		attributes.set(WeaponAttributes::charge);
		attributes.set(WeaponAttributes::no_reload);
		specifications.charge_multiplier = in_data["gameplay"]["attributes"]["charge_multiplier"].as<float>();
	}
	auto laser = in_data["class_package"]["laser"];
	if (laser) {
		auto att = util::BitFlags<world::LaserAttributes>{};
		if (!enemy) { att.set(world::LaserAttributes::player); }
		m_laser = LaserSpecifications{
			static_cast<world::LaserType>(laser["type"].as<int>()), laser["active"].as<int>(), laser["cooldown"].as<int>(), laser["size"].as<float>(), laser["damage"].as<float>(), att,
		};
	}

	// chargeable weapons start with no juice
	if (attributes.test(WeaponAttributes::charge)) { cooldowns.reload.start(); }

	// audio
	m_audio.shoot = static_cast<audio::Weapon>(in_data["audio"]["shoot"].as<int>());
	metadata.audio_tag = "shot_" + std::string{tag};
	NANI_LOG_DEBUG(m_logger, "Weapon audio tag: {}", metadata.audio_tag);

	set_parameters({in_data["visual"]["texture_lookup"].as<int>(), 1, 32, -1});
}

void Weapon::update(automa::ServiceProvider& svc, world::Map& map, Direction to_direction) {
	ammo.update();
	if (attributes.test(WeaponAttributes::automatic) && has_flag_set(WeaponFlags::firing) && cooldowns.cooldown.running() && !ammo.empty()) { svc.soundboard.repeat_sound(get_audio_tag(), 1, get_barrel_point()); }
	if (attributes.test(WeaponAttributes::charge) && has_flag_set(WeaponFlags::charging) && !cooldowns.reload.is_complete()) { svc.soundboard.repeat_sound("charge_" + metadata.tag, 1, get_barrel_point()); }
	if (attributes.test(WeaponAttributes::charge) && has_flag_set(WeaponFlags::charging) && cooldowns.reload.is_almost_complete()) {
		svc.soundboard.play_sound("charge_complete_" + metadata.tag, get_barrel_point());
		set_flag(WeaponFlags::overdrive);
	}
	if (has_flag_set(WeaponFlags::overdrive)) { svc.soundboard.repeat_sound("overdrive_" + metadata.tag, 1, get_barrel_point()); }
	m_modifiers.speed_multiplier = 1.f;
	m_modifiers.damage_multiplier = 1.f;
	if (attributes.test(WeaponAttributes::charge) && has_flag_set(WeaponFlags::released)) {
		set_flag(WeaponFlags::overdrive, false);
		m_modifiers.speed_multiplier = std::max(minimum_speed_multiplier_v, cooldowns.reload.get_inverse_normalized() * specifications.speed_multiplier);
		m_modifiers.damage_multiplier = std::max(minimum_damage_multiplier_v, cooldowns.reload.get_inverse_normalized() * specifications.charge_multiplier);
		if (cooldowns.reload.is_complete()) {
			if (m_laser) {
				map.spawn_laser(svc, get_barrel_point(), CardinalDirection{firing_direction}, *m_laser);
				svc.ticker.freeze_frame(6);
			} else {
				shoot(svc, map, get_barrel_point());
			}
			svc.soundboard.play_sound("release_" + metadata.tag, get_barrel_point());
		} else if (!cooldowns.cooldown.running()) {
			shoot(svc, map, get_barrel_point());
			svc.soundboard.play_sound(get_audio_tag(), get_barrel_point());
		}
		cooldowns.reload.start();
	}
	cooldowns.reload.set_native_time(specifications.reload_time * m_modifiers.reload_multiplier);
	tick();
	if (cooldowns.reload.is_almost_complete() && projectile.get_team() == Team::nani && !attributes.test(WeaponAttributes::no_reload)) { svc.soundboard.flags.arms.set(audio::Arms::reload); }
	if (cooldowns.reload.is_almost_complete()) { ammo.refill(); }
	if (!attributes.test(WeaponAttributes::charge)) { cooldowns.reload.update(); }
	if (attributes.test(WeaponAttributes::charge) && has_flag_set(WeaponFlags::charging)) { cooldowns.reload.update(); }
	cooldowns.shoot_effect.update();

	set_orientation(to_direction);
	cooldowns.cooldown.update();
	physical.steering.seek(physical.physics, {}, 0.01f);
	physical.physics.simple_update();
	physical.final_position = visual.position + physical.physics.position;
}

void Weapon::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Animatable::set_position(physical.final_position - cam);
	if (attributes.test(WeaponAttributes::charge)) {
		auto progress = static_cast<int>(std::round(cooldowns.reload.get_inverse_normalized() * 3.f));
		set_channel(progress);
	} else {
		set_channel(cooldowns.shoot_effect.is_complete() ? 0 : cooldowns.shoot_effect.halfway() ? 2 : 1);
	}
	win.draw(*this);
	if (svc.greyblock_mode()) {
		sf::RectangleShape box{};
		box.setSize({2.f, 2.f});
		box.setOrigin({1.f, 1.f});
		box.setPosition(get_barrel_point() - cam);
		box.setFillColor(colors::fucshia);
		win.draw(box);
		box.setPosition(visual.position - cam);
		box.setFillColor(colors::goldenrod);
		win.draw(box);
	} else {
		// visual.sprite.render(svc, win, cam);
	}
}

void Weapon::render_ui(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f position) {
	auto gun_dimensions = sf::Vector2<int>{48, 48};
	visual.ui.setTextureRect(sf::IntRect({metadata.id * gun_dimensions.x, 0}, gun_dimensions));
	visual.ui.setPosition(position);
	win.draw(visual.ui);
}

void Weapon::equip() { flags.state.set(WeaponState::equipped); }

void Weapon::unequip() { flags.state.reset(WeaponState::equipped); }

void Weapon::unlock() { flags.state.set(WeaponState::unlocked); }

void Weapon::lock() { flags.state.reset(WeaponState::unlocked); }

void Weapon::shoot() {
	cooldowns.cooldown.start(specifications.cooldown_time);
	if (attributes.test(WeaponAttributes::charge)) {
		cooldowns.reload = util::Cooldown{static_cast<int>(specifications.reload_time * m_modifiers.reload_multiplier)};
		cooldowns.reload.start();
	} else if (!cooldowns.reload.running()) {
		cooldowns.reload = util::Cooldown{static_cast<int>(specifications.reload_time * m_modifiers.reload_multiplier)};
		cooldowns.reload.start();
	}

	active_projectiles.update();
	ammo.use();
	physical.physics.apply_force(firing_direction.get_vector() * -1.f);
	cooldowns.shoot_effect.start();
	if (!attributes.test(WeaponAttributes::automatic) && !attributes.test(WeaponAttributes::charge)) { m_services->soundboard.play_sound(get_audio_tag(), get_barrel_point()); }
}

void Weapon::shoot(automa::ServiceProvider& svc, world::Map& map) {
	if (attributes.test(WeaponAttributes::charge)) { return; }
	shoot(svc, map, get_barrel_point());
}

void Weapon::shoot(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f target) {
	shoot();
	map.spawn_projectile_at(svc, *this, get_barrel_point(), target, m_modifiers.speed_multiplier, m_modifiers.damage_multiplier);
}

void Weapon::decrement_projectiles() { active_projectiles.update(-1); }

bool Weapon::is_equipped() const { return flags.state.test(WeaponState::equipped); }

bool Weapon::is_unlocked() const { return flags.state.test(WeaponState::unlocked); }

bool Weapon::cooling_down() const { return !cooldowns.cooldown.is_complete(); }

bool Weapon::can_shoot() const { return !cooling_down() && ammo.get_count() > 0; }

void Weapon::set_position(sf::Vector2f pos) { visual.position = pos; }

void Weapon::force_position(sf::Vector2f pos) {
	set_position(pos);
	physical.physics.position = {};
}

void Weapon::set_barrel_point(sf::Vector2f point) { offsets.gameplay.barrel = point; }

void Weapon::set_orientation(Direction to_direction) {
	auto right_scale = constants::f_scale_vec;
	auto left_scale = sf::Vector2f{-constants::f_scale_factor, constants::f_scale_factor};
	auto neutral_rotation{0.0f};
	auto right_offset = sf::Vector2f{-offsets.render.global.x, -offsets.render.global.y};
	auto left_offset = sf::Vector2f{offsets.render.global.x, -offsets.render.global.y};
	auto right_barrel_offset = sf::Vector2f{offsets.render.barrel.x, offsets.render.barrel.y};
	auto left_barrel_offset = sf::Vector2f{-offsets.render.barrel.x, offsets.render.barrel.y};
	auto const& position = physical.final_position;
	set_rotation(sf::degrees(neutral_rotation));
	switch (firing_direction.lnr) {
	case LNR::right:
		set_scale(right_scale);
		offsets.gameplay.barrel = position + right_offset + right_barrel_offset;
		break;
	case LNR::left:
		set_scale(left_scale);
		offsets.gameplay.barrel = position + left_offset + left_barrel_offset;
		break;
	default: break;
	}
	switch (firing_direction.und) {
	case UND::up:
		to_direction.right() ? Drawable::rotate(sf::degrees(-90)) : Drawable::rotate(sf::degrees(90));
		if (to_direction.left()) { offsets.gameplay.barrel = {position.x - left_offset.y - left_barrel_offset.y, position.y + left_offset.x + left_barrel_offset.x}; }
		if (to_direction.right()) { offsets.gameplay.barrel = {position.x + right_offset.y + right_barrel_offset.y, position.y - right_offset.x - right_barrel_offset.x}; }
		firing_direction.neutralize_lr();
		break;
	case UND::down:
		to_direction.lnr == LNR::right ? Drawable::rotate(sf::degrees(90)) : Drawable::rotate(sf::degrees(-90));
		if (to_direction.left()) { offsets.gameplay.barrel = {position.x + left_offset.y + left_barrel_offset.y, position.y - left_offset.x - left_barrel_offset.x}; }
		if (to_direction.right()) { offsets.gameplay.barrel = {position.x - right_offset.y - right_barrel_offset.y, position.y + right_offset.x + right_barrel_offset.x}; }
		firing_direction.neutralize_lr();
		break;
	default: break;
	}
	projectile.set_firing_direction(firing_direction);
}

void Weapon::set_team(Team team) { projectile.set_team(team); }

void Weapon::set_firing_direction(Direction to_direction) { firing_direction = to_direction; }

void Weapon::reduce_reload_time(float percentage) {
	auto amount = static_cast<int>(static_cast<float>(cooldowns.reload.get_native_time()) * percentage);
	amount = std::clamp(amount, 0, cooldowns.reload.get() - 1);
	cooldowns.reload.update(amount);
}

void Weapon::reset() { active_projectiles.start(); }

} // namespace fornani::arms
