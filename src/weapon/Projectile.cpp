
#include "Projectile.hpp"
#include "../setup/LookupTables.hpp"
#include "../setup/ServiceLocator.hpp"

namespace arms {

Projectile::Projectile() {
	physics = components::PhysicsComponent({1.0f, 1.0f}, 1.0f);
	physics.velocity.x = stats.speed;
	seed();
};

Projectile::Projectile(int id) {

	auto const& in_data = svc::dataLocator.get().weapon["weapons"][id]["projectile"];

	type = index_to_type.at(id);

	stats.base_damage = in_data["attributes"]["base_damage"].as<float>();
	stats.range = in_data["attributes"]["range"].as<int>();
	stats.speed = in_data["attributes"]["speed"].as<int>();
	stats.variance = in_data["attributes"]["variance"].as<float>();
	stats.stun_time = in_data["attributes"]["stun_time"].as<float>();
	stats.knockback = in_data["attributes"]["knockback"].as<float>();
	stats.boomerang = (bool)in_data["attributes"]["boomerang"].as_bool();
	stats.persistent = (bool)in_data["attributes"]["persistent"].as_bool();
	stats.transcendent = (bool)in_data["attributes"]["transcendent"].as_bool();
	stats.constrained = (bool)in_data["attributes"]["constrained"].as_bool();
	stats.range_variance = in_data["attributes"]["range_variance"].as<float>();
	stats.acceleration_factor = in_data["attributes"]["acceleration_factor"].as<float>();
	stats.dampen_factor = in_data["attributes"]["dampen_factor"].as<float>();

	anim.num_sprites = in_data["animation"]["num_sprites"].as<int>();
	anim.num_frames = in_data["animation"]["num_frames"].as<int>();
	anim.framerate = in_data["animation"]["framerate"].as<int>();

	sf::Vector2<float> dim{};
	dim.x = in_data["dimensions"]["x"].as<float>();
	dim.y = in_data["dimensions"]["y"].as<float>();

	render_type = anim.num_sprites > 1 ? RENDER_TYPE::MULTI_SPRITE : RENDER_TYPE::SINGLE_SPRITE;

	physics = components::PhysicsComponent({1.0f, 1.0f}, 1.0f);
	stats.attractor_force = 0.025f;
	attractor = vfx::Attractor(physics.position, flcolor::goldenrod, stats.attractor_force);
	attractor.collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.993f, 0.993f}, 1.0f);
	attractor.collider.physics.maximum_velocity = {60.f, 60.f};
	physics.velocity.x = stats.speed;

	anim::Parameters params = {0, anim.num_frames, anim.framerate, -1};
	animation.set_params(params);

	bounding_box.dimensions = dim;
	max_dimensions = dim;

	state.set(ProjectileState::initialized);
	cooldown.start(40);
	seed();
	set_sprite();
}

void Projectile::update() {

	cooldown.update();

	//tomahawk-specific stuff
	if (stats.boomerang) {
		attractor.set_target_position(svc::playerLocator.get().apparent_position);
		attractor.update();
		physics.position = attractor.collider.physics.position;
		svc::soundboardLocator.get().weapon.set(lookup::gun_sound.at(type)); //repeat sound
		if (attractor.collider.bounding_box.SAT(svc::playerLocator.get().collider.bounding_box) && cooldown.is_complete()) {
			destroy(true);
			svc::soundboardLocator.get().weapon.set(audio::Weapon::tomahawk_catch);
		} // destroy when player catches it
	}

	constrain_hitbox_at_barrel();
	if (state.test(ProjectileState::destruction_initiated)) { constrain_hitbox_at_destruction_point(); }
	if (state.test(ProjectileState::destruction_initiated) && !stats.constrained) { destroy(true); }

	physics.update_euler();

	if (direction.lr == dir::LR::left) {
		bounding_box.set_position(shape::Shape::Vec{physics.position.x, physics.position.y - bounding_box.dimensions.y / 2});
	} else if (direction.lr == dir::LR::right) {
		bounding_box.set_position(shape::Shape::Vec{physics.position.x - bounding_box.dimensions.x, physics.position.y - bounding_box.dimensions.y / 2});
	} else if (direction.und == dir::UND::up) {
		bounding_box.set_position(shape::Shape::Vec{physics.position.x - bounding_box.dimensions.x / 2, physics.position.y});
	} else if (direction.und == dir::UND::down) {
		bounding_box.set_position(shape::Shape::Vec{physics.position.x - bounding_box.dimensions.x / 2, physics.position.y - bounding_box.dimensions.y});
	}

	position_history.push_back(physics.position);
	if (position_history.size() > history_limit) { position_history.pop_front(); }

	// animation
	animation.update();

	if (direction.lr == dir::LR::left || direction.lr == dir::LR::right) {
		if (abs(physics.position.x - fired_point.x) >= stats.range) { destroy(false); }
	} else {
		if (abs(physics.position.y - fired_point.y) >= stats.range) { destroy(false); }
	}
}

void Projectile::render(sf::RenderWindow& win, sf::Vector2<float>& campos) {

	// this is the right idea but needs to be refactored and generalized
	if (render_type == RENDER_TYPE::MULTI_SPRITE) {
		for (auto& sprite : sp_proj) {
			constrain_sprite_at_barrel(sprite, campos);
			win.draw(sprite);
			svc::counterLocator.get().at(svc::draw_calls)++;
		}

	} else if (render_type == RENDER_TYPE::SINGLE_SPRITE) {
		if (!sp_proj.empty()) {

			// get UV coords (only one row of sprites is supported)
			int u = (int)(animation.get_frame() * max_dimensions.x);
			int v = 0;
			sp_proj.at(0).setTextureRect(sf::IntRect({u, v}, {(int)max_dimensions.x, (int)max_dimensions.y}));

			if (stats.boomerang) { sp_proj.at(0).setPosition(attractor.collider.physics.position - campos); }
			constrain_sprite_at_barrel(sp_proj.at(0), campos);
			if (state.test(ProjectileState::destruction_initiated)) { constrain_sprite_at_destruction_point(sp_proj.at(0), campos); }

			// proj bounding box for debug
			box.setSize(bounding_box.dimensions);
			if (state.test(ProjectileState::destruction_initiated)) {
				box.setFillColor(sf::Color{255, 255, 60, 160});
			} else {
				box.setFillColor(sf::Color{255, 255, 255, 160});
			}
			box.setPosition(bounding_box.position.x - campos.x, bounding_box.position.y - campos.y);

			if (svc::globalBitFlagsLocator.get().test(svc::global_flags::greyblock_state)) {
				attractor.render(win, campos);
				win.draw(box);
				svc::counterLocator.get().at(svc::draw_calls)++;
			} else {
				win.draw(sp_proj.at(0));
				svc::counterLocator.get().at(svc::draw_calls)++;
			}
		}
	}
}

void Projectile::destroy(bool completely) {
	// destruction_point = point;
	if (completely) {
		state.set(ProjectileState::destroyed);
		return;
	}

	if (!state.test(ProjectileState::destruction_initiated)) {
		if (direction.lr == dir::LR::left || direction.und == dir::UND::up) {
			destruction_point = bounding_box.position;
		} else {
			destruction_point = bounding_box.position + bounding_box.dimensions;
		}
		state.set(ProjectileState::destruction_initiated);
	}

	stats.base_damage = 0;
}

void Projectile::seed() {
	util::Random r{};
	stats.range += r.random_range(-stats.range_variance, stats.range_variance);
	float var = r.random_range_float(-stats.variance, stats.variance);
	switch (direction.lr) {
	case dir::LR::left: physics.velocity = {-stats.speed + (var / 2), var}; break;
	case dir::LR::right: physics.velocity = {stats.speed + (var / 2), var}; break;
	}
	switch (direction.und) {
	case dir::UND::up: physics.velocity = {var, -stats.speed + (var / 2)}; break;
	case dir::UND::down: physics.velocity = {var, stats.speed + (var / 2)}; break;
	}
}

void Projectile::set_sprite() {

	for (int i = 0; i < anim.num_sprites; ++i) { sp_proj.push_back(sf::Sprite()); }

	for (auto& sprite : sp_proj) {
		set_orientation(sprite);
		// sprite.setTextureRect(sf::IntRect({0, 0}, static_cast<sf::Vector2<int> >(bounding_box.dimensions)));
		sprite.setTexture(lookup::type_to_texture_ref.at(type));
	}
}

void Projectile::set_orientation(sf::Sprite& sprite) {
	// assume right
	sprite.setScale({1.0f, 1.0f});
	sprite.setRotation(0.0f);
	sprite.setOrigin(0, 0);

	switch (direction.lr) {
	case dir::LR::left: sprite.scale({-1.0f, 1.0f}); break;
	case dir::LR::right: break;
	}
	switch (direction.und) {
	case dir::UND::up: sprite.rotate(-90); break;
	case dir::UND::down: sprite.rotate(90); break;
	}
}

void Projectile::set_position(sf::Vector2<float>& pos) {
	physics.position = pos;
	bounding_box.position = pos;
	fired_point = pos;
}

void Projectile::set_boomerang_speed() {
	attractor.collider.physics.velocity.x = direction.lr == dir::LR::left ? -stats.speed : (direction.lr == dir::LR::right ? stats.speed : 0.f);
	attractor.collider.physics.velocity.y = direction.und == dir::UND::up ? -stats.speed : (direction.und == dir::UND::down ? stats.speed : 0.f);
}

void Projectile::sync_position() { attractor.collider.physics.position = fired_point; }

void Projectile::constrain_sprite_at_barrel(sf::Sprite& sprite, sf::Vector2<float>& campos) {
	if (!stats.constrained) { return; }
	if (direction.lr != dir::LR::neutral) {
		if (abs(physics.position.x - fired_point.x) < max_dimensions.x) {
			int width = abs(physics.position.x - fired_point.x);
			sprite.setTextureRect(sf::IntRect({(int)(max_dimensions.x - width), 0}, {width, (int)max_dimensions.y}));
			bounding_box.dimensions.x = width;
		} else {
			sprite.setTextureRect(sf::IntRect({0, 0}, {(int)(bounding_box.dimensions.x), (int)(bounding_box.dimensions.y)}));
			bounding_box.dimensions.x = max_dimensions.x;
		}
		if (direction.lr == dir::LR::right) {
			sprite.setPosition(bounding_box.position.x - campos.x, bounding_box.position.y - campos.y);
		} else if (direction.lr == dir::LR::left) {
			sprite.setPosition(bounding_box.position.x + bounding_box.dimensions.x - campos.x, bounding_box.position.y - campos.y);
		}

	} else {
		bounding_box.dimensions.x = max_dimensions.y;
		if (abs(physics.position.y - fired_point.y) < max_dimensions.x) {
			int height = abs(physics.position.y - fired_point.y);
			sprite.setTextureRect(sf::IntRect({(int)(max_dimensions.x - height), 0}, {height, (int)max_dimensions.y}));
			bounding_box.dimensions.y = height;
		} else {
			sprite.setTextureRect(sf::IntRect({0, 0}, {(int)(max_dimensions.x), (int)(max_dimensions.y)}));
			bounding_box.dimensions.y = max_dimensions.x;
		}
		if (direction.und == dir::UND::up) {
			sprite.setPosition(bounding_box.position.x - campos.x, bounding_box.position.y + bounding_box.dimensions.y - campos.y);
		} else if (direction.und == dir::UND::down) {
			sprite.setPosition(bounding_box.position.x + bounding_box.dimensions.x - campos.x, bounding_box.position.y - campos.y);
		}
	}
}

void Projectile::constrain_sprite_at_destruction_point(sf::Sprite& sprite, sf::Vector2<float>& campos) {
	if (!stats.constrained) { return; }
	if (direction.lr != dir::LR::neutral) {
		if (direction.lr == dir::LR::left) {
			float rear = bounding_box.dimensions.x + physics.position.x;
			int width = abs(rear - destruction_point.x);
			sprite.setTextureRect(sf::IntRect({0, 0}, {width, (int)max_dimensions.y}));
			bounding_box.dimensions.x = width;
			bounding_box.position.x = destruction_point.x;
			sprite.setPosition(bounding_box.position.x + bounding_box.dimensions.x - campos.x, bounding_box.position.y - campos.y);
			if (rear <= destruction_point.x) { destroy(true); }
		} else {
			float rear = physics.position.x - bounding_box.dimensions.x;
			int width = abs(rear - destruction_point.x);
			sprite.setTextureRect(sf::IntRect({0, 0}, {width, (int)max_dimensions.y}));
			bounding_box.dimensions.x = width;
			bounding_box.position.x = destruction_point.x - width;
			sprite.setPosition(bounding_box.position.x - campos.x, bounding_box.position.y - campos.y);
			if (rear >= destruction_point.x) { destroy(true); }
		}

	} else {
		if (direction.und == dir::UND::up) {
			float rear = bounding_box.dimensions.y + physics.position.y;
			int height = abs(rear - destruction_point.y);
			sprite.setTextureRect(sf::IntRect({0, 0}, {height, (int)max_dimensions.y}));
			bounding_box.dimensions.y = height;
			bounding_box.position.y = destruction_point.y;
			sprite.setPosition(bounding_box.position.x - campos.x, bounding_box.position.y + bounding_box.dimensions.y - campos.y);
			if (rear <= destruction_point.y) { destroy(true); }
		} else {
			float rear = physics.position.y - bounding_box.dimensions.y;
			int height = abs(rear - destruction_point.y);
			sprite.setTextureRect(sf::IntRect({0, 0}, {height, (int)max_dimensions.y}));
			bounding_box.dimensions.y = height;
			bounding_box.position.y = destruction_point.y - height;
			sprite.setPosition(bounding_box.position.x + bounding_box.dimensions.x - campos.x, bounding_box.position.y - campos.y);
			if (rear >= destruction_point.y) { destroy(true); }
		}
	}
}

void Projectile::constrain_hitbox_at_barrel() {
	if (!stats.constrained) { return; }
	if (direction.lr != dir::LR::neutral) {
		if (abs(physics.position.x - fired_point.x) < max_dimensions.x) {
			int width = abs(physics.position.x - fired_point.x);
			bounding_box.dimensions.x = width;
		} else {
			bounding_box.dimensions.x = max_dimensions.x;
		}
		if (direction.lr == dir::LR::right) {
		} else if (direction.lr == dir::LR::left) {
		}

	} else {
		bounding_box.dimensions.x = max_dimensions.y;
		if (abs(physics.position.y - fired_point.y) < max_dimensions.x) {
			int height = abs(physics.position.y - fired_point.y);
			bounding_box.dimensions.y = height;
		} else {
			bounding_box.dimensions.y = max_dimensions.x;
		}
		if (direction.und == dir::UND::up) {
		} else if (direction.und == dir::UND::down) {
		}
	}
}

void Projectile::constrain_hitbox_at_destruction_point() {
	if (!stats.constrained) { return; }
	if (direction.lr != dir::LR::neutral) {
		if (direction.lr == dir::LR::left) {
			float rear = bounding_box.dimensions.x + physics.position.x;
			int width = abs(rear - destruction_point.x);
			bounding_box.dimensions.x = width;
			bounding_box.position.x = destruction_point.x;
			if (rear <= destruction_point.x) { destroy(true); }
		} else {
			float rear = physics.position.x - bounding_box.dimensions.x;
			int width = abs(rear - destruction_point.x);
			bounding_box.dimensions.x = width;
			bounding_box.position.x = destruction_point.x - width;
			if (rear >= destruction_point.x) { destroy(true); }
		}

	} else {
		if (direction.und == dir::UND::up) {
			float rear = bounding_box.dimensions.y + physics.position.y;
			int height = abs(rear - destruction_point.y);
			bounding_box.dimensions.y = height;
			bounding_box.position.y = destruction_point.y;
			if (rear <= destruction_point.y) { destroy(true); }
		} else {
			float rear = physics.position.y - bounding_box.dimensions.y;
			int height = abs(rear - destruction_point.y);
			bounding_box.dimensions.y = height;
			bounding_box.position.y = destruction_point.y - height;
			if (rear >= destruction_point.y) { destroy(true); }
		}
	}
}

} // namespace arms
