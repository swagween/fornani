
#include "Weapon.hpp"
#include "../setup/ServiceLocator.hpp"

namespace arms {

Weapon::Weapon(int id) : id(id) {

	auto const& in_data = svc::dataLocator.get().weapon["weapons"][id];

	label = in_data["label"].as_string();
	type = index_to_type.at(id);

	sprite_dimensions.x = in_data["dimensions"]["x"].as<int>();
	sprite_dimensions.y = in_data["dimensions"]["y"].as<int>();
	gun_offset.x = in_data["gun_offset"]["x"].as<int>();
	gun_offset.y = in_data["gun_offset"]["y"].as<int>();

	attributes.barrel_position.at(0) = in_data["barrel_point"]["x"].as<float>();
	attributes.barrel_position.at(1) = in_data["barrel_point"]["y"].as<float>();

	attributes.automatic = (bool)in_data["attributes"]["automatic"].as_bool();
	attributes.rate = in_data["attributes"]["rate"].as<int>();
	attributes.cooldown_time = in_data["attributes"]["cooldown_time"].as<int>();
	attributes.recoil = in_data["attributes"]["recoil"].as<float>();
	attributes.ui_color = (COLOR_CODE)in_data["attributes"]["ui_color"].as<int>();

	spray_behavior.rate = in_data["spray"]["particle"]["rate"].as<int>();
	spray_behavior.rate_variance = in_data["spray"]["particle"]["rate_variance"].as<int>();
	spray_behavior.expulsion_force = in_data["spray"]["particle"]["expulsion_force"].as<float>();
	spray_behavior.expulsion_variance = in_data["spray"]["particle"]["expulsion_variance"].as<float>();
	spray_behavior.cone = in_data["spray"]["particle"]["cone"].as<float>();
	spray_behavior.grav = in_data["spray"]["particle"]["grav"].as<float>();
	spray_behavior.grav_variance = in_data["spray"]["particle"]["grav_variance"].as<float>();
	spray_behavior.x_friction = in_data["spray"]["particle"]["x_friction"].as<float>();
	spray_behavior.y_friction = in_data["spray"]["particle"]["y_friction"].as<float>();

	spray_stats.lifespan = in_data["spray"]["emitter"]["lifespan"].as<int>();
	spray_stats.lifespan_variance = in_data["spray"]["emitter"]["lifespan_variance"].as<int>();
	spray_stats.particle_lifespan = in_data["spray"]["emitter"]["particle_lifespan"].as<int>();
	spray_stats.particle_lifespan_variance = in_data["spray"]["emitter"]["particle_lifespan_variance"].as<int>();

	spray = vfx::Emitter(spray_behavior, spray_stats, spray_color.at(type), spray_color.at(type));

	projectile = Projectile(id);
	attributes.boomerang = projectile.stats.boomerang;
}

void Weapon::update() {
	active_projectiles = std::clamp(active_projectiles, 0, INT_MAX);
	set_orientation();
	if (cooling_down()) { --cooldown_counter; }
	if (cooldown_counter < 0) {
		flags.reset(GunState::cooling_down);
		cooldown_counter = 0;
	}
	if (cooldown_counter > 0) { flags.set(GunState::cooling_down); }
	sf::Vector2<float> p_pos = {svc::playerLocator.get().apparent_position.x + gun_offset.x,
								svc::playerLocator.get().apparent_position.y + svc::playerLocator.get().sprite_offset.y + gun_offset.y - svc::playerLocator.get().collider.dimensions.y / 2.f};
	set_position(p_pos);
}

void Weapon::render(sf::RenderWindow& win, sf::Vector2<float>& campos) {

	// nani threw it, so don't render it in her hand
	if (attributes.boomerang && active_projectiles == attributes.rate) { return; }

	// set sprite position
	sp_gun.setPosition(sprite_position.x - campos.x, sprite_position.y - campos.y);

	if (svc::globalBitFlagsLocator.get().test(svc::global_flags::greyblock_state)) {
		// fire point debug
		sf::RectangleShape box{};
		box.setPosition(barrel_point.x - campos.x - 1, barrel_point.y - campos.y - 1);
		box.setFillColor(flcolor::fucshia);
		box.setSize(sf::Vector2<float>{2.0f, 2.0f});
		win.draw(box);
		
	} else {
		win.draw(sp_gun);
		
	}
}

void Weapon::equip() { flags.set(GunState::equipped); }
void Weapon::unequip() { flags.reset(GunState::equipped); }
void Weapon::unlock() { flags.set(GunState::unlocked); }
void Weapon::lock() { flags.reset(GunState::unlocked); }

void Weapon::shoot() { cooldown(); }

void Weapon::cooldown() {
	flags.set(GunState::cooling_down);
	cooldown_counter = attributes.cooldown_time;
}

bool Weapon::is_equipped() const { return flags.test(GunState::equipped); }
bool Weapon::is_unlocked() const { return flags.test(GunState::unlocked); }

bool Weapon::cooling_down() const { return flags.test(GunState::cooling_down); }

bool Weapon::can_shoot() const { return !cooling_down() && !(active_projectiles >= attributes.rate); }

void Weapon::set_position(sf::Vector2<float> pos) { sprite_position = pos; }

void Weapon::set_orientation() {

	// flip the sprite based on the player's direction
	sf::Vector2<float> right_scale = {1.0f, 1.0f};
	sf::Vector2<float> left_scale = {-1.0f, 1.0f};
	// rotate the sprite based on the player's direction
	float neutral_rotation{0.0f};
	float up_rotation{-90.f};
	float down_rotation{90.f};
	// start from default
	sp_gun.setRotation(neutral_rotation);
	sp_gun.setScale(right_scale);

	firing_direction = svc::playerLocator.get().controller.direction;

	switch (firing_direction.lr) {
	case dir::LR::right: barrel_point.x -= 2.0f * attributes.barrel_position.at(1); break;
	case dir::LR::left:
		barrel_point.x += 2.0f * attributes.barrel_position.at(1);
		sp_gun.scale(-1.0f, 1.0f);
		break;
	default: break;
	}
	switch (firing_direction.und) {
	case dir::UND::up:
		svc::playerLocator.get().controller.direction.lr == dir::LR::right ? sp_gun.rotate(-90) : sp_gun.rotate(90);
		barrel_point = {sprite_position.x + attributes.barrel_position.at(1), sprite_position.y - attributes.barrel_position.at(0)};
		firing_direction.neutralize_lr();
		break;
	case dir::UND::down:
		svc::playerLocator.get().controller.direction.lr == dir::LR::right ? sp_gun.rotate(90) : sp_gun.rotate(-90);
		barrel_point = {sprite_position.x + sprite_dimensions.y - attributes.barrel_position.at(1) - sprite_dimensions.y, sprite_position.y + sprite_dimensions.x};
		firing_direction.neutralize_lr();
		break;
	case dir::UND::neutral:
		switch (firing_direction.lr) {
		case dir::LR::right: barrel_point = {sprite_position.x + attributes.barrel_position.at(0), sprite_position.y + attributes.barrel_position.at(1)}; break;
		case dir::LR::left: barrel_point = {sprite_position.x - attributes.barrel_position.at(0), sprite_position.y + attributes.barrel_position.at(1)}; break;
		}
		break;
	default: break;
	}
	projectile.direction = firing_direction;
}

int Weapon::get_id() { return id; }

} // namespace arms
