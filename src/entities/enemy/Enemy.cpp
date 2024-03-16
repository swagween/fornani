#include "Enemy.hpp"
#include "../../service/ServiceProvider.hpp"

namespace enemy {
Enemy::Enemy(automa::ServiceProvider& svc, std::string_view label) : entity::Entity(svc), label(label) {
	auto const& in_data = svc.data.enemy[label];
	auto const& in_metadata = in_data["metadata"];
	auto const& in_physical = in_data["physical"];
	auto const& in_attributes = in_data["attributes"];
	auto const& in_animation = in_data["animation"];
	auto const& in_general = in_data["general"];

	metadata.id = in_metadata["id"].as<int>();
	metadata.variant = in_metadata["variant"].as_string();

	sprite_dimensions.x = in_physical["sprite_dimensions"][0].as<int>();
	sprite_dimensions.y = in_physical["sprite_dimensions"][1].as<int>();
	spritesheet_dimensions.x = in_physical["spritesheet_dimensions"][0].as<int>();
	spritesheet_dimensions.y = in_physical["spritesheet_dimensions"][1].as<int>();

	// TODO: load hurtboxes and colliders

	physical.alert_range.x = in_physical["alert_range"][0].as<float>();
	physical.alert_range.y = in_physical["alert_range"][1].as<float>();
	physical.hostile_range.x = in_physical["hostile_range"][0].as<float>();
	physical.hostile_range.y = in_physical["hostile_range"][1].as<float>();

	attributes.base_damage = in_attributes["base_damage"].as<int>();
	attributes.base_hp = in_attributes["base_hp"].as<float>();
	attributes.loot_multiplier = in_attributes["loot_multiplier"].as<float>();
	attributes.speed = in_attributes["speed"].as<float>();

	// TODO: load in all the animation data and map them to a set of parameters
	// let's add this function to services

	if (in_general["mobile"].as_bool()) { general_flags.set(GeneralFlags::mobile); }
	if (in_general["gravity"].as_bool()) { general_flags.set(GeneralFlags::gravity); }
	if (general_flags.test(GeneralFlags::gravity)) { collider.stats.GRAV = 0.f; }

	sprite.setTexture(svc.assets.texture_lookup.at(label));
	drawbox.setOutlineColor(svc.styles.colors.ui_white);
	drawbox.setOutlineThickness(-1);
}

void Enemy::update(automa::ServiceProvider& svc) {
	collider.update();
	animation.update();
}

void Enemy::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	drawbox.setPosition(collider.physics.position + sprite_offset - cam);
	sprite.setPosition(collider.physics.position + sprite_offset - cam);
	if (svc.debug_flags.test(automa::DebugFlags::greyblock_mode)) {
		win.draw(drawbox);
	} else {
		win.draw(sprite);
	}
}

} // namespace enemy