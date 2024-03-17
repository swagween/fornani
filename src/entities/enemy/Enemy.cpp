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

	dimensions.x = in_physical["dimensions"][0].as<float>();
	dimensions.y = in_physical["dimensions"][1].as<float>();

	collider = shape::Collider(dimensions);
	collider.sync_components();
	collider.physics.set_global_friction(in_physical["friction"].as<float>());
	collider.stats.GRAV = in_physical["gravity"].as<float>();

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
	attributes.drop_range.x = in_attributes["drop_range"][0].as<int>();
	attributes.drop_range.y = in_attributes["drop_range"][1].as<int>();
	// TODO: load in all the animation data and map them to a set of parameters
	// let's add this function to services
	anim::Parameters params{};
	params.duration = in_animation["duration"].as<int>();
	params.framerate = in_animation["framerate"].as<int>();
	animation.set_params(params);

	health.set_max(attributes.base_hp);

	if (in_general["mobile"].as_bool()) { flags.general.set(GeneralFlags::mobile); }
	if (in_general["gravity"].as_bool()) { flags.general.set(GeneralFlags::gravity); }
	if (!flags.general.test(GeneralFlags::gravity)) { collider.stats.GRAV = 0.f; }

	sprite.setTexture(svc.assets.texture_lookup.at(label));
	drawbox.setSize({(float)sprite_dimensions.x, (float)sprite_dimensions.y});
	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineColor(svc.styles.colors.ui_white);
	drawbox.setOutlineThickness(-1);
}

void Enemy::update(automa::ServiceProvider& svc, world::Map& map) {
	collider.update();
	collider.detect_map_collision(map);
	collider.reset();
	collider.reset_ground_flags();
	collider.physics.acceleration = {};
	animation.update();
	health.update(svc, map);
	// get UV coords
	if (spritesheet_dimensions.y != 0) {
		int u = (int)(animation.get_frame() / spritesheet_dimensions.y) * sprite_dimensions.x;
		int v = (int)(animation.get_frame() % spritesheet_dimensions.y) * sprite_dimensions.y;
		sprite.setTextureRect(sf::IntRect({u, v}, {sprite_dimensions.x, sprite_dimensions.y}));
	}
	sprite.setOrigin((float)sprite_dimensions.x / 2.f, (float)dimensions.y / 2.f);
}

void Enemy::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	drawbox.setOrigin(sprite.getOrigin());
	drawbox.setPosition(collider.physics.position + sprite_offset - cam);
	sprite.setPosition(collider.physics.position + sprite_offset - cam);
	if (svc.debug_flags.test(automa::DebugFlags::greyblock_mode)) {
		health.drawbox.setPosition(collider.physics.position + sprite_offset - cam);
		win.draw(drawbox);
		collider.render(win, cam);
		health.render(svc, win, cam);
	} else {
		win.draw(sprite);
	}
}

} // namespace enemy