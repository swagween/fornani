
#include "fornani/weapon/Weapon.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::arms {

Weapon::Weapon(automa::ServiceProvider& svc, int id, bool enemy)
	: metadata{.id = id, .label = enemy ? svc.data.enemy_weapon["weapons"][id]["metadata"]["label"].as_string() : svc.data.weapon["weapons"][id]["metadata"]["label"].as_string()},
	  projectile(svc, enemy ? svc.data.enemy_weapon["weapons"][id]["metadata"]["tag"].as_string() : svc.data.weapon["weapons"][id]["metadata"]["tag"].as_string(), id, *this, enemy),
	  visual{.sprite{sf::Sprite{svc.assets.get_texture("guns")}}, .ui{sf::Sprite{svc.assets.get_texture("inventory_guns")}}} {

	auto const& in_data = enemy ? svc.data.enemy_weapon["weapons"][id] : svc.data.weapon["weapons"][id];

	// metadata
	metadata.description = in_data["metadata"]["description"].as_string();

	// visual
	visual.dimensions = {in_data["visual"]["dimensions"][0].as<int>(), in_data["visual"]["dimensions"][1].as<int>()};
	offsets.render.global = {in_data["visual"]["offsets"]["global"][0].as<float>(), in_data["visual"]["offsets"]["global"][1].as<float>()};
	offsets.render.barrel = {in_data["visual"]["offsets"]["barrel"][0].as<float>(), in_data["visual"]["offsets"]["barrel"][1].as<float>()};
	visual.color = static_cast<UIColor>(in_data["visual"]["ui"]["color"].as<int>());
	emitter.dimensions = {in_data["visual"]["spray"]["dimensions"][0].as<float>(), in_data["visual"]["spray"]["dimensions"][1].as<float>()};
	try {
		emitter.color = svc.styles.spray_colors.at(metadata.label);
	} catch (std::out_of_range) { emitter.color = colors::white; }

	emitter.type = in_data["visual"]["spray"]["type"].as_string().data(); // secondary emitter

	if (in_data["visual"]["secondary_spray"]) {
		secondary_emitter = EmitterAttributes();
		secondary_emitter.value().dimensions.x = in_data["visual"]["secondary_spray"]["dimensions"][0].as<float>();
		secondary_emitter.value().dimensions.y = in_data["visual"]["secondary_spray"]["dimensions"][1].as<float>();
		try {
			secondary_emitter.value().color = svc.styles.spray_colors.at(metadata.label);
		} catch (std::out_of_range) { secondary_emitter.value().color = colors::white; }
		secondary_emitter.value().type = in_data["visual"]["secondary_spray"]["type"].as_string();
	}
	visual.texture_lookup = in_data["visual"]["texture_lookup"].as<int>() * 16;
	visual.sprite.setTextureRect(sf::IntRect{{0, visual.texture_lookup}, visual.dimensions}); // TODO: allow for custom gun animations
	visual.sprite.setOrigin(offsets.render.global);

	// gameplay
	ammo.set_max(in_data["gameplay"]["attributes"]["ammo"].as<int>());
	cooldowns.reload = util::Cooldown{in_data["gameplay"]["attributes"]["reload"].as<int>()};
	specifications.multishot = in_data["gameplay"]["attributes"]["multishot"].as<int>();
	specifications.cooldown_time = in_data["gameplay"]["attributes"]["cooldown_time"].as<int>();
	specifications.recoil = in_data["gameplay"]["attributes"]["recoil"].as<float>();
	if (static_cast<bool>(in_data["gameplay"]["attributes"]["automatic"].as_bool())) { attributes.set(WeaponAttributes::automatic); }

	// audio
	m_audio.shoot = static_cast<audio::Weapon>(in_data["audio"]["shoot"].as<int>());
}

void Weapon::update(automa::ServiceProvider& svc, Direction to_direction) {
	ammo.update();
	if (cooldowns.reload.is_almost_complete() && projectile.get_team() == Team::nani) { svc.soundboard.flags.arms.set(audio::Arms::reload); }
	if (cooldowns.reload.is_almost_complete()) { ammo.refill(); }
	cooldowns.reload.update();

	set_orientation(to_direction);
	cooldowns.cooldown.update();
	physical.steering.seek(physical.physics, {}, 0.01f);
	physical.physics.simple_update();
}

void Weapon::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	physical.final_position = visual.position + physical.physics.position;
	visual.sprite.setPosition(physical.final_position - cam);
	win.draw(visual.sprite);
	if (svc.greyblock_mode()) {
		sf::RectangleShape box{};
		box.setSize({2.f, 2.f});
		box.setOrigin({1.f, 1.f});
		box.setPosition(offsets.gameplay.barrel - cam);
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
	if (!cooldowns.reload.running()) { cooldowns.reload.start(); }
	active_projectiles.update();
	ammo.use();
	physical.physics.apply_force(firing_direction.get_vector() * -1.f);
}

void Weapon::shoot(automa::ServiceProvider& svc, world::Map& map) {
	shoot();
	map.spawn_projectile_at(svc, *this, get_barrel_point());
	svc.soundboard.flags.weapon.set(m_audio.shoot);
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
	auto right_scale = sf::Vector2f{1.f, 1.f};
	auto left_scale = sf::Vector2f{-1.f, 1.f};
	auto neutral_rotation{0.0f};
	auto up_rotation{-90.f};
	auto down_rotation{90.f};
	auto right_offset = sf::Vector2f{-offsets.render.global.x, -offsets.render.global.y};
	auto left_offset = sf::Vector2f{offsets.render.global.x, -offsets.render.global.y};
	auto right_barrel_offset = sf::Vector2f{offsets.render.barrel.x, offsets.render.barrel.y};
	auto left_barrel_offset = sf::Vector2f{-offsets.render.barrel.x, offsets.render.barrel.y};
	auto const& position = physical.final_position;
	visual.sprite.setRotation(sf::degrees(neutral_rotation));
	switch (firing_direction.lnr) {
	case LNR::right:
		visual.sprite.setScale(right_scale);
		offsets.gameplay.barrel = position + right_offset + right_barrel_offset;
		break;
	case LNR::left:
		visual.sprite.setScale(left_scale);
		offsets.gameplay.barrel = position + left_offset + left_barrel_offset;
		break;
	default: break;
	}
	switch (firing_direction.und) {
	case UND::up:
		to_direction.right() ? visual.sprite.rotate(sf::degrees(-90)) : visual.sprite.rotate(sf::degrees(90));
		if (to_direction.left()) { offsets.gameplay.barrel = {position.x - left_offset.y - left_barrel_offset.y, position.y + left_offset.x + left_barrel_offset.x}; }
		if (to_direction.right()) { offsets.gameplay.barrel = {position.x + right_offset.y + right_barrel_offset.y, position.y - right_offset.x - right_barrel_offset.x}; }
		firing_direction.neutralize_lr();
		break;
	case UND::down:
		to_direction.lnr == LNR::right ? visual.sprite.rotate(sf::degrees(90)) : visual.sprite.rotate(sf::degrees(-90));
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

void Weapon::reset() { active_projectiles.start(); }

} // namespace fornani::arms
