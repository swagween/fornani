
#include "Weapon.hpp"
#include "../setup/ServiceLocator.hpp"

namespace arms {

	Weapon::Weapon(int id) : id(id) {
	
		label = svc::dataLocator.get().weapon["weapons"][0]["label"].as_string();

		attributes.automatic = svc::dataLocator.get().weapon["weapons"][0]["attributes"]["automatic"].as<bool>();
		attributes.boomerang = svc::dataLocator.get().weapon["weapons"][0]["attributes"]["boomerang"].as<bool>();
		attributes.rate = svc::dataLocator.get().weapon["weapons"][0]["attributes"]["rate"].as<int>();
		attributes.cooldown_time = svc::dataLocator.get().weapon["weapons"][0]["attributes"]["cooldown_time"].as<int>();
		attributes.recoil = svc::dataLocator.get().weapon["weapons"][0]["attributes"]["recoil"].as<float>();
		//attributes.ui_color = svc::dataLocator.get().weapon["weapons"][0]["attributes"]["ui_color"].as<int>();

		projectile = Projectile(id);

	
	}

Weapon::Weapon(int id, std::string lbl, WEAPON_TYPE weapon_type, WeaponAttributes const& wa, ProjectileStats const& ps, vfx::ElementBehavior const spr, ProjectileAnimation const& pa, RENDER_TYPE rt, sf::Vector2<int> dim,
			   sf::Vector2<float> proj_dim)
	: id(id), label(lbl), type(weapon_type), attributes(wa), sprite_dimensions(dim) {

	projectile = Projectile(ps, components::PhysicsComponent(), pa, weapon_type, rt, proj_dim);
	spray = vfx::Emitter(spr, burst, spray_color.at(type));
	barrel_point = {sprite_position.x + 18, sprite_position.y + 1};

}

void Weapon::update() {
	set_orientation();
	if (cooling_down()) { --cooldown_counter; }
	if (cooldown_counter < 0) {
		flags.reset(GunState::cooling_down);
		cooldown_counter = 0;
	}
	if (cooldown_counter > 0) { flags.set(GunState::cooling_down); }
}

void Weapon::render(sf::RenderWindow& win, sf::Vector2<float>& campos) {
	sf::Vector2<float> p_pos = {svc::playerLocator.get().apparent_position.x, svc::playerLocator.get().apparent_position.y + svc::playerLocator.get().sprite_offset.y + 8};
	sf::Vector2<float> h_pos = svc::playerLocator.get().hand_position;
	sp_gun.setPosition(p_pos.x - campos.x, p_pos.y - campos.y);
	set_position(p_pos);

	// fire point debug
	sf::RectangleShape box{};
	box.setPosition(barrel_point.x - campos.x - 1, barrel_point.y - campos.y - 1);
	box.setFillColor(flcolor::fucshia);
	box.setSize(sf::Vector2<float>{2.0f, 2.0f});

	if (svc::globalBitFlagsLocator.get().test(svc::global_flags::greyblock_state)) {
		win.draw(box);
		svc::counterLocator.get().at(svc::draw_calls)++;
	} else {
		win.draw(sp_gun);
		svc::counterLocator.get().at(svc::draw_calls)++;
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
	}

	projectile.direction = firing_direction;
}

int Weapon::get_id() { return id; }

} // namespace arms
