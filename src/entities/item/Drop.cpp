
#include "Drop.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../../setup/ServiceLocator.hpp"

namespace item {

Drop::Drop(automa::ServiceProvider& svc, std::string_view key, float probability) : sparkler(svc, drop_dimensions, flcolor::ui_white, "drop") {

	collider = shape::Collider(drop_dimensions);
	collider.sync_components();
	collider.physics.elasticity = 1.0f;

	sprite_dimensions.x = svc.data.drop[key]["sprite_dimensions"][0].as<float>();
	sprite_dimensions.y = svc.data.drop[key]["sprite_dimensions"][1].as<float>();
	spritesheet_dimensions.x = svc.data.drop[key]["spritesheet_dimensions"][0].as<int>();
	spritesheet_dimensions.y = svc.data.drop[key]["spritesheet_dimensions"][1].as<int>();

	parameters.type = svc.data.drop[key]["type"].as<int>() == 0 ? DropType::heart : DropType::orb;

	collider.physics.set_global_friction(svc.data.drop[key]["friction"].as<float>());
	collider.stats.GRAV = svc.data.drop[key]["gravity"].as<float>();

	num_sprites = svc.data.drop[key]["animation"]["num_sprites"].as<int>();
	animation.params.duration = svc.data.drop[key]["animation"]["duration"].as<int>();
	animation.params.framerate = svc.data.drop[key]["animation"]["framerate"].as<int>();
	animation.start();
	animation.refresh();

	// randomly seed the animation start frame so drops in the same loot animate out of sync
	animation.current_frame = svc::randomLocator.get().random_range(0, animation.params.duration - 1);

	int rand_cooldown_offset = svc::randomLocator.get().random_range(0, 50);
	lifespan.start(2500 + rand_cooldown_offset);
	seed(probability);
	set_value();
	set_texture(svc);

	sparkler.set_dimensions(collider.bounding_box.dimensions);
}

void Drop::seed(float probability) {

	float random_sample = svc::randomLocator.get().random_range_float(0.0f, 1.0f);

	if (random_sample < probability * priceless_constant) {
		rarity = priceless;
	} else if (random_sample < probability * rare_constant) {
		rarity = rare;
	} else if (random_sample < probability * uncommon_constant) {
		rarity = uncommon;
	} else {
		rarity = common;
	}
}

void Drop::set_value() {

	// heart
	if (parameters.type == DropType::heart) {
		if (rarity == priceless || rarity == rare) {
			value = 4;
		} else {
			value = 1;
		}
		return;
	}

	// orb
	switch (rarity) {
	case priceless: value = 100; break;
	case rare: value = 50; break;
	case uncommon: value = 10; break;
	case common: value = 1; break;
	}
}

void Drop::set_texture(automa::ServiceProvider& svc) {

	switch (parameters.type) {
	case DropType::heart: sprite.setTexture(svc.assets.t_heart); break;
	case DropType::orb: sprite.setTexture(svc.assets.t_orb); break;
	}
}

void Drop::update(automa::ServiceProvider& svc, world::Map& map) {

	collider.update(svc);
	collider.detect_map_collision(map);
	collider.reset();
	collider.reset_ground_flags();
	collider.physics.acceleration = {};

	lifespan.update();
	afterlife.update();

	animation.update();

	sparkler.update(svc);
	sparkler.set_position(collider.bounding_box.position);

	sprite_offset = drop_dimensions - sprite_dimensions;

	if (parameters.type == DropType::heart) {
		auto frame = animation.get_frame();
		auto y = rarity == priceless || rarity == rare ? 1.f : 0.f;
		auto rect = sf::IntRect({(int)(frame * sprite_dimensions.x), (int)(y * sprite_dimensions.y)}, static_cast<sf::Vector2<int>>(sprite_dimensions));
		sprite.setTextureRect(rect);
	}
	if (parameters.type == DropType::orb) {
		auto frame = animation.get_frame();
		auto y = rarity == priceless ? 3.f : (rarity == rare ? 2.f : (rarity == uncommon ? 1.f : 0.f));
		auto rect = sf::IntRect({(int)(frame * sprite_dimensions.x), (int)(y * sprite_dimensions.y)}, static_cast<sf::Vector2<int>>(sprite_dimensions));
		sprite.setTextureRect(rect);
	}
}

void Drop::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> campos) {

	if (svc::globalBitFlagsLocator.get().test(svc::global_flags::greyblock_state)) {
		collider.render(win, campos);
	} else {
		sprite.setPosition(collider.physics.position + sprite_offset - campos);
		if (!is_inactive() && !is_completely_gone() && (lifespan.get_cooldown() > 500 || (lifespan.get_cooldown() / 20) % 2 == 0)) {
			win.draw(sprite);
		}
		if (parameters.type == DropType::heart) { sparkler.render(svc, win, campos); }
		sparkler.render(svc, win, campos);
	}
}

void Drop::set_position(sf::Vector2<float> pos) {
	collider.physics.position = pos;
	sparkler.set_position(pos);
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

shape::Collider& Drop::get_collider() { return collider; }

DropType Drop::get_type() const { return parameters.type; }

int Drop::get_value() const { return value; }

bool Drop::is_completely_gone() const { return afterlife.is_complete() && lifespan.is_complete(); }

bool Drop::is_inactive() const { return lifespan.is_complete() && !afterlife.is_complete(); }

} // namespace item
