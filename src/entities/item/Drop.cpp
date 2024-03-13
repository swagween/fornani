
#include "Drop.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../../setup/ServiceLocator.hpp"

namespace item {

Drop::Drop(automa::ServiceProvider& svc, std::string_view key, float probability) {

	collider.bounding_box.dimensions = drop_dimensions;

	sprite_dimensions.x = svc.data.drop[key]["sprite_dimensions"][0].as<float>();
	sprite_dimensions.y = svc.data.drop[key]["sprite_dimensions"][1].as<float>();
	spritesheet_dimensions.x = svc.data.drop[key]["spritesheet_dimensions"][0].as<int>();
	spritesheet_dimensions.y = svc.data.drop[key]["spritesheet_dimensions"][1].as<int>();

	parameters.type = svc.data.drop[key]["type"].as<int>() == 0 ? DropType::heart : DropType::orb;

	num_sprites = svc.data.drop[key]["animation"]["num_sprites"].as<int>();
	animation.params.duration = svc.data.drop[key]["animation"]["duration"].as<int>();
	animation.params.framerate = svc.data.drop[key]["animation"]["framerate"].as<int>();
	animation.start();
	animation.refresh();

	seed(probability);
	set_value();
	set_texture(svc);
}

void Drop::seed(float probability) {

	float rarity = svc::randomLocator.get().random_range_float(0.0f, 1.0f);
	if (probability * priceless_constant < rarity) {
		rarity = priceless;
	} else if (probability * rare_constant < rarity) {
		rarity = rare;
	} else if (probability * uncommon_constant < rarity) {
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

void Drop::update() {
	collider.update();
	animation.update();
	sprite_offset = drop_dimensions - sprite_dimensions;

	int x{};
	int y{};
	if (parameters.type == DropType::heart) {
		auto frame = animation.get_frame();
		if (rarity == priceless || rarity == rare) { y = 1.f; }
		auto rect = sf::IntRect({(int)(x * sprite_dimensions.x), (int)(y * sprite_dimensions.y)}, static_cast<sf::Vector2<int>>(sprite_dimensions));
		sprite.setTextureRect(rect);
	}
	if (parameters.type == DropType::orb) {
		auto frame = animation.get_frame();
		y = rarity == priceless ? 3.f : (rarity == rare ? 2.f : (rarity == uncommon ? 1.f : 0.f));
		auto rect = sf::IntRect({(int)(x * sprite_dimensions.x), (int)(y * sprite_dimensions.y)}, static_cast<sf::Vector2<int>>(sprite_dimensions));
		sprite.setTextureRect(rect);
	}
	//std::cout << collider.physics.position.y << "\n";
}

void Drop::render(sf::RenderWindow& win, sf::Vector2<float> campos) {

	sprite.setPosition(collider.physics.position - sprite_offset - campos);
	win.draw(sprite);
}

void Drop::set_position(sf::Vector2<float> pos) { collider.physics.position = pos; }

shape::Collider& Drop::get_collider() { return collider; }

} // namespace item
