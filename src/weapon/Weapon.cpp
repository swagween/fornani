#include "Weapon.hpp"
#include "../service/ServiceProvider.hpp"

namespace arms {

Weapon::Weapon(automa::ServiceProvider& svc, int id)
	: metadata{.id = id, .label = svc.data.weapon["weapons"][id]["metadata"]["label"].as_string()}, projectile(svc, svc.data.weapon["weapons"][id]["metadata"]["label"].as_string(), id, *this) {

	auto const& in_data = svc.data.weapon["weapons"][id];

	// metadata
	metadata.description = in_data["metadata"]["description"].as_string();

	// visual
	visual.dimensions = {in_data["visual"]["dimensions"][0].as<int>(), in_data["visual"]["dimensions"][1].as<int>()};
	visual.sprite.setTexture(svc.assets.t_gun);
	offsets.render.global = {in_data["visual"]["offsets"]["global"][0].as<float>(), in_data["visual"]["offsets"]["global"][1].as<float>()};
	offsets.render.stock = {in_data["visual"]["offsets"]["stock"][0].as<int>(), in_data["visual"]["offsets"]["stock"][1].as<int>()};
	offsets.render.barrel = {in_data["gameplay"]["offsets"]["barrel"][0].as<float>(), in_data["gameplay"]["offsets"]["barrel"][1].as<float>()};
	visual.color = static_cast<UIColor>(in_data["visual"]["ui"]["color"].as<int>());
	emitter.dimensions = {in_data["visual"]["spray"]["dimensions"][0].as<float>(), in_data["visual"]["spray"]["dimensions"][1].as<float>()};
	try {
		emitter.color = svc.styles.spray_colors.at(metadata.label);
	} catch (std::out_of_range) { emitter.color = svc.styles.colors.white; }
	emitter.type = in_data["visual"]["spray"]["type"].as_string(); // secondary emitter
	if (in_data["visual"]["secondary_spray"]) {
		secondary_emitter = EmitterAttributes();
		secondary_emitter.value().dimensions.x = in_data["visual"]["secondary_spray"]["dimensions"][0].as<float>();
		secondary_emitter.value().dimensions.y = in_data["visual"]["secondary_spray"]["dimensions"][1].as<float>();
		try {
			secondary_emitter.value().color = svc.styles.spray_colors.at(metadata.label);
		} catch (std::out_of_range) { secondary_emitter.value().color = svc.styles.colors.white; }
		secondary_emitter.value().type = in_data["visual"]["secondary_spray"]["type"].as_string();
	}

	// gameplay
	offsets.gameplay.barrel = {in_data["gameplay"]["offsets"]["barrel"][0].as<float>(), in_data["gameplay"]["offsets"]["barrel"][1].as<float>()};
	ammo.set_max(in_data["gameplay"]["attributes"]["ammo"].as<int>());
	cooldowns.reload = util::Cooldown{in_data["gameplay"]["attributes"]["reload"].as<int>()};
	cooldowns.down_time = util::Cooldown{cooldowns.reload};
	specifications.multishot = in_data["gameplay"]["attributes"]["multishot"].as<int>();
	specifications.cooldown_time = in_data["gameplay"]["attributes"]["cooldown_time"].as<int>();
	specifications.recoil = in_data["gameplay"]["attributes"]["recoil"].as<float>();
	if (static_cast<bool>(in_data["gameplay"]["attributes"]["automatic"].as_bool())) { attributes.set(WeaponAttributes::automatic); }

	visual.texture_lookup = in_data["visual"]["texture_lookup"].as<int>() * 16;
	visual.ui.setTexture(svc.assets.t_guns);
	visual.sprite.setTextureRect(sf::IntRect{{0, metadata.id * visual.texture_lookup}, visual.dimensions}); // TODO: allow for custom gun animations
}

void Weapon::update(automa::ServiceProvider& svc, dir::Direction to_direction) {
	ammo.update();
	if ((ammo.empty() || !cooldowns.down_time.running()) && !cooldowns.reload.running() && !ammo.full()) { cooldowns.reload.start(); }
	if (cooldowns.reload.is_almost_complete()) { svc.soundboard.flags.arms.set(audio::Arms::reload); }
	if (cooldowns.reload.is_almost_complete()) { ammo.refill(); }
	cooldowns.reload.update();
	if (!cooldowns.reload.running()) { cooldowns.down_time.update(); }

	set_orientation(to_direction);
	cooldowns.cooldown.update();
}

void Weapon::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (svc.greyblock_mode()) {
		// fire point debug
		sf::RectangleShape box{};
		box.setSize({2.f, 2.f});
		box.setOrigin(1.f, 1.f);
		box.setPosition(offsets.gameplay.barrel - cam);
		box.setFillColor(svc.styles.colors.fucshia);
		win.draw(box);
	} else {
		//visual.sprite.render(svc, win, cam);
		visual.sprite.setPosition(visual.position - cam);
		win.draw(visual.sprite);
	}
}

void Weapon::render_ui(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> position) {
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
	cooldowns.down_time.start();
	active_projectiles.update();
	ammo.use();
}

void Weapon::decrement_projectiles() { active_projectiles.update(-1); }

bool Weapon::is_equipped() const { return flags.state.test(WeaponState::equipped); }

bool Weapon::is_unlocked() const { return flags.state.test(WeaponState::unlocked); }

bool Weapon::cooling_down() const { return !cooldowns.cooldown.is_complete(); }

bool Weapon::can_shoot() const { return !cooling_down() && ammo.get_count() > 0; }

void Weapon::set_position(sf::Vector2<float> pos) { visual.position = pos + offsets.render.global; }

void Weapon::set_barrel_point(sf::Vector2<float> point) { offsets.gameplay.barrel = point; }

void Weapon::set_orientation(dir::Direction to_direction) {

	sf::Vector2<float> right_scale = {1.0f, 1.0f};
	sf::Vector2<float> left_scale = {-1.0f, 1.0f};
	float neutral_rotation{0.0f};
	float up_rotation{-90.f};
	float down_rotation{90.f};

	// start from default
	auto& sp_gun = visual.sprite;
	sp_gun.setRotation(neutral_rotation);
	sp_gun.setScale(right_scale);

	switch (firing_direction.lr) {
	case dir::LR::right:
		offsets.gameplay.barrel.x -= 2.0f * offsets.render.barrel.y;
		offsets.gameplay.barrel.x += offsets.render.stock.x;
		break;
	case dir::LR::left:
		offsets.gameplay.barrel.x += 2.0f * offsets.render.barrel.y;
		offsets.gameplay.barrel.x -= offsets.render.stock.x;
		sp_gun.scale(-1.0f, 1.0f);
		break;
	default: break;
	}
	switch (firing_direction.und) {
	case dir::UND::up:
		to_direction.right() ? sp_gun.rotate(-90) : sp_gun.rotate(90);
		offsets.gameplay.barrel = {visual.position.x + offsets.render.barrel.y, visual.position.y - offsets.render.barrel.x};
		if (to_direction.left()) { offsets.gameplay.barrel.x = visual.position.x - offsets.render.barrel.y + 1.f; }
		if (to_direction.right()) { offsets.gameplay.barrel.x = visual.position.x + offsets.render.barrel.y - 1.f; }
		firing_direction.neutralize_lr();
		offsets.gameplay.barrel.y += offsets.render.stock.x;
		break;
	case dir::UND::down:
		to_direction.lr == dir::LR::right ? sp_gun.rotate(90) : sp_gun.rotate(-90);
		offsets.gameplay.barrel = {visual.position.x - offsets.render.barrel.y, visual.position.y + visual.dimensions.x};
		if (to_direction.right()) { offsets.gameplay.barrel.x = visual.position.x + offsets.render.barrel.y - 1.f; }
		if (to_direction.left()) { offsets.gameplay.barrel.x = visual.position.x - offsets.render.barrel.y + 1.f; }
		firing_direction.neutralize_lr();
		offsets.gameplay.barrel.y -= offsets.render.stock.x;
		break;
	case dir::UND::neutral:
		switch (firing_direction.lr) {
		case dir::LR::right: offsets.gameplay.barrel = {visual.position.x + offsets.render.barrel.x - offsets.render.stock.x, visual.position.y + offsets.render.barrel.y}; break;
		case dir::LR::left: offsets.gameplay.barrel = {visual.position.x - offsets.render.barrel.x + offsets.render.stock.x, visual.position.y + offsets.render.barrel.y}; break;
		}
		break;
	default: break;
	}
	projectile.set_firing_direction(firing_direction);
}

void Weapon::set_team(Team team) { projectile.set_team(team); }

void Weapon::set_firing_direction(dir::Direction to_direction) { firing_direction = to_direction; }

void Weapon::reset() { active_projectiles.start(); }

} // namespace arms
