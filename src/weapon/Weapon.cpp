#include "Weapon.hpp"
#include "../service/ServiceProvider.hpp"

namespace arms {

Weapon::Weapon(automa::ServiceProvider& svc, int id) : label(svc.data.weapon["weapons"][id]["label"].as_string()), id(id), projectile(svc, svc.data.weapon["weapons"][id]["label"].as_string(), id, *this) {

	auto const& in_data = svc.data.weapon["weapons"][id];

	// label = in_data["label"].as_string();
	type = static_cast<WEAPON_TYPE>(id);

	metadata.description = in_data["description"].as_string();

	sprite_dimensions.x = in_data["dimensions"]["x"].as<int>();
	sprite_dimensions.y = in_data["dimensions"]["y"].as<int>();
	gun_offset.x = in_data["gun_offset"]["x"].as<float>();
	gun_offset.y = in_data["gun_offset"]["y"].as<float>();

	attributes.back_offset = in_data["attributes"]["back_offset"].as<int>();
	attributes.barrel_position.at(0) = in_data["barrel_point"]["x"].as<float>();
	attributes.barrel_position.at(1) = in_data["barrel_point"]["y"].as<float>();

	ammo.set_max(in_data["attributes"]["ammo"].as<int>());
	cooldowns.reload = util::Cooldown{in_data["attributes"]["reload"].as<int>()};
	cooldowns.down_time = util::Cooldown{cooldowns.reload};
	attributes.automatic = static_cast<bool>(in_data["attributes"]["automatic"].as_bool());
	attributes.grenade = static_cast<bool>(in_data["attributes"]["grenade"].as_bool());
	attributes.rate = in_data["attributes"]["rate"].as<int>();
	attributes.multishot = in_data["attributes"]["multishot"].as<int>();
	attributes.cooldown_time = in_data["attributes"]["cooldown_time"].as<int>();
	attributes.recoil = in_data["attributes"]["recoil"].as<float>();
	attributes.ui_color = static_cast<COLOR_CODE>(in_data["attributes"]["ui_color"].as<int>());

	emitter.dimensions.x = in_data["spray"]["dimensions"][0].as<float>();
	emitter.dimensions.y = in_data["spray"]["dimensions"][1].as<float>();

	try {
		emitter.color = svc.styles.spray_colors.at(label);
	} catch (std::out_of_range) { emitter.color = svc.styles.colors.white; }
	emitter.type = in_data["spray"]["type"].as_string();

	// secondary emitter
	if (in_data["secondary_spray"]) {
		secondary_emitter = EmitterAttributes();
		secondary_emitter.value().dimensions.x = in_data["secondary_spray"]["dimensions"][0].as<float>();
		secondary_emitter.value().dimensions.y = in_data["secondary_spray"]["dimensions"][1].as<float>();
		try {
			secondary_emitter.value().color = svc.styles.spray_colors.at(label);
		} catch (std::out_of_range) { secondary_emitter.value().color = svc.styles.colors.white; }
		secondary_emitter.value().type = in_data["secondary_spray"]["type"].as_string();
	}

	auto texture_lookup = in_data["texture_lookup"].as<int>() * 16;
	attributes.boomerang = projectile.stats.boomerang;
	sp_gun.setTexture(svc.assets.t_gun);
	sp_gun_back.setTexture(svc.assets.t_gun);
	sp_gun_back.setOrigin({static_cast<float>(attributes.back_offset), 0.f});
	sp_gun.setTextureRect(sf::IntRect({attributes.back_offset, texture_lookup}, {sprite_dimensions.x - attributes.back_offset, sprite_dimensions.y}));
	sp_gun_back.setTextureRect(sf::IntRect({0, texture_lookup}, {attributes.back_offset, sprite_dimensions.y}));
	sprites.ui.setTexture(svc.assets.t_guns);
}

void Weapon::update(automa::ServiceProvider& svc, dir::Direction to_direction) {

	// ammo
	ammo.update();
	if ((ammo.empty() || !cooldowns.down_time.running()) && !cooldowns.reload.running() && !ammo.full()) { cooldowns.reload.start(); }
	if (cooldowns.reload.is_almost_complete()) { svc.soundboard.flags.arms.set(audio::Arms::reload); }
	if (cooldowns.reload.is_almost_complete()) { ammo.refill(); }
	cooldowns.reload.update();
	if (!cooldowns.reload.running()) { cooldowns.down_time.update(); }

	set_orientation(to_direction);
	cooldown.update();
	if (cooldown.is_complete()) {
		flags.reset(GunState::cooling_down);
	} else {
		flags.set(GunState::cooling_down);
	}
}

void Weapon::render_back(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float>& campos) {
	if (attributes.boomerang && active_projectiles.get_count() == attributes.rate) { return; }
	if (!svc.greyblock_mode()) { win.draw(sp_gun_back); }
}

void Weapon::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float>& campos) {

	// nani threw it, so don't render it in her hand
	if (attributes.boomerang && active_projectiles.get_count() == attributes.rate) { return; }

	// set sprite position
	sp_gun.setPosition(sprite_position.x - campos.x, sprite_position.y - campos.y);

	if (svc.greyblock_mode()) {
		// fire point debug
		sf::RectangleShape box{};
		box.setOrigin(1.f, 1.f);
		box.setPosition(barrel_point.x - campos.x, barrel_point.y - campos.y);
		box.setFillColor(svc.styles.colors.fucshia);
		box.setSize(sf::Vector2<float>{2.0f, 2.0f});
		win.draw(box);
	} else {
		win.draw(sp_gun);
	}
}

void Weapon::render_ui(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> position) {
	auto gun_dimensions = sf::Vector2<int>{48, 48};
	sprites.ui.setTextureRect(sf::IntRect({id * gun_dimensions.x, 0}, gun_dimensions));
	sprites.ui.setPosition(position);
	win.draw(sprites.ui);
}

void Weapon::equip() { flags.set(GunState::equipped); }
void Weapon::unequip() { flags.reset(GunState::equipped); }
void Weapon::unlock() { flags.set(GunState::unlocked); }
void Weapon::lock() { flags.reset(GunState::unlocked); }

void Weapon::shoot() {
	cooldown.start(attributes.cooldown_time);
	cooldowns.down_time.start();
	active_projectiles.update();
	ammo.use();
}

void Weapon::decrement_projectiles() { active_projectiles.update(-1); }

bool Weapon::is_equipped() const { return flags.test(GunState::equipped); }

bool Weapon::is_unlocked() const { return flags.test(GunState::unlocked); }

bool Weapon::cooling_down() const { return !cooldown.is_complete(); }

bool Weapon::can_shoot() const { return !cooling_down() && !(active_projectiles.get_count() >= attributes.rate) && ammo.get_count() > 0; }

void Weapon::set_position(sf::Vector2<float> pos) { sprite_position = pos; }

void Weapon::set_orientation(dir::Direction to_direction) {

	sf::Vector2<float> right_scale = {1.0f, 1.0f};
	sf::Vector2<float> left_scale = {-1.0f, 1.0f};
	float neutral_rotation{0.0f};
	float up_rotation{-90.f};
	float down_rotation{90.f};

	// start from default
	sp_gun.setRotation(neutral_rotation);
	sp_gun.setScale(right_scale);

	switch (firing_direction.lr) {
	case dir::LR::right:
		barrel_point.x -= 2.0f * attributes.barrel_position.at(1);
		barrel_point.x += attributes.back_offset;
		break;
	case dir::LR::left:
		barrel_point.x += 2.0f * attributes.barrel_position.at(1);
		barrel_point.x -= attributes.back_offset;
		sp_gun.scale(-1.0f, 1.0f);
		break;
	default: break;
	}
	switch (firing_direction.und) {
	case dir::UND::up:
		to_direction.right() ? sp_gun.rotate(-90) : sp_gun.rotate(90);
		barrel_point = {sprite_position.x + attributes.barrel_position.at(1), sprite_position.y - attributes.barrel_position.at(0)};
		if (to_direction.left()) { barrel_point.x = sprite_position.x - attributes.barrel_position.at(1) + 1.f; }
		if (to_direction.right()) { barrel_point.x = sprite_position.x + attributes.barrel_position.at(1) - 1.f; }
		firing_direction.neutralize_lr();
		barrel_point.y += attributes.back_offset;
		break;
	case dir::UND::down:
		to_direction.lr == dir::LR::right ? sp_gun.rotate(90) : sp_gun.rotate(-90);
		barrel_point = {sprite_position.x - attributes.barrel_position.at(1), sprite_position.y + sprite_dimensions.x};
		if (to_direction.right()) { barrel_point.x = sprite_position.x + attributes.barrel_position.at(1) - 1.f; }
		if (to_direction.left()) { barrel_point.x = sprite_position.x - attributes.barrel_position.at(1) + 1.f; }
		firing_direction.neutralize_lr();
		barrel_point.y -= attributes.back_offset;
		break;
	case dir::UND::neutral:
		switch (firing_direction.lr) {
		case dir::LR::right: barrel_point = {sprite_position.x + attributes.barrel_position.at(0) - attributes.back_offset, sprite_position.y + attributes.barrel_position.at(1)}; break;
		case dir::LR::left: barrel_point = {sprite_position.x - attributes.barrel_position.at(0) + attributes.back_offset, sprite_position.y + attributes.barrel_position.at(1)}; break;
		}
		break;
	default: break;
	}
	projectile.direction = firing_direction;
	sp_gun_back.setRotation(sp_gun.getRotation());
	sp_gun_back.setPosition(sp_gun.getPosition());
	sp_gun_back.setScale(sp_gun.getScale());
}

void Weapon::reset() { active_projectiles.start(); }

} // namespace arms
