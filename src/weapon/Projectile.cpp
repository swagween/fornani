//
//  Projectile.cpp
//  arms
//
//

#include "Projectile.hpp"
#include "../setup/ServiceLocator.hpp"
#include "../setup/LookupTables.hpp"

namespace arms {

	Projectile::Projectile() {
		physics = components::PhysicsComponent();
		physics.velocity.x = stats.speed;
		seed();
	};
	Projectile::Projectile(ProjectileStats s, components::PhysicsComponent p, ProjectileAnimation a, WEAPON_TYPE t, RENDER_TYPE rt, sf::Vector2<float> dim) : stats(s), physics(p), anim(a), type(t), render_type(rt) {
		physics.velocity.x = stats.speed;
		bounding_box.dimensions = dim;
		max_dimensions = dim;
		state.set(ProjectileState::initialized);
		seed();
		set_sprite();

		//set projectile colors
		if (spray_color.at(t) == flcolor::periwinkle) {
			colors.push_back(sf::Color{ 164, 133, 255 });
			colors.push_back(sf::Color{ 206, 170, 255 });
			colors.push_back(sf::Color{ 236, 201, 255 });
			colors.push_back(sf::Color{ 243, 239, 255 });
		}
		if (spray_color.at(t) == flcolor::fucshia) {
			colors.push_back(sf::Color{ 191, 0, 105 });
			colors.push_back(sf::Color{ 203, 39, 130 });
			colors.push_back(sf::Color{ 227, 27, 116 });
			colors.push_back(sf::Color{ 255, 0, 83 });
		}
	}

	void Projectile::update() {

		physics.update_euler();
		if (dir == FIRING_DIRECTION::LEFT) {
			bounding_box.set_position(shape::Shape::Vec{ physics.position.x, physics.position.y - bounding_box.dimensions.y / 2 });
		} else if (dir == FIRING_DIRECTION::RIGHT) {
			bounding_box.set_position(shape::Shape::Vec{ physics.position.x - bounding_box.dimensions.x, physics.position.y - bounding_box.dimensions.y / 2 });
		} else if (dir == FIRING_DIRECTION::UP) {
			bounding_box.set_position(shape::Shape::Vec{ physics.position.x - bounding_box.dimensions.x / 2, physics.position.y });
		} else if (dir == FIRING_DIRECTION::DOWN) {
			bounding_box.set_position(shape::Shape::Vec{ physics.position.x - bounding_box.dimensions.x / 2, physics.position.y - bounding_box.dimensions.y });
		}
		position_history.push_back(physics.position);
		if (position_history.size() > history_limit) {
			position_history.pop_front();
		}

		dt = svc::clockLocator.get().tick_rate;

		auto new_time = Clock::now();
		Time frame_time = std::chrono::duration_cast<Time>(new_time - current_time);

		if (frame_time.count() > svc::clockLocator.get().frame_limit) {
			frame_time = Time{ svc::clockLocator.get().frame_limit };
		}
		current_time = new_time;
		accumulator += frame_time;

		int integrations = 0;
		while (accumulator >= dt) {

			//animation
			if (curr_frame % anim.framerate == 0) {
				anim_frame++;
			}
			if (anim_frame >= anim.num_frames) { anim_frame = 0; }
			curr_frame++;

			accumulator -= dt;
			++integrations;
		}

		if (dir == FIRING_DIRECTION::LEFT || dir == FIRING_DIRECTION::RIGHT) {
			if (abs(physics.position.x - fired_point.x) >= stats.range) { destroy(false); }
		} else {
			if (abs(physics.position.y - fired_point.y) >= stats.range) { destroy(false); }
		}

	}

	void Projectile::render(sf::RenderWindow& win, sf::Vector2<float>& campos) {

		//this is the right idea but needs to be refactored and generalized
		if (render_type == RENDER_TYPE::MULTI_SPRITE) {
			for (auto& sprite : sp_proj) {
				constrain_at_barrel(sprite, campos);
				win.draw(sprite);
				svc::counterLocator.get().at(svc::draw_calls)++;
			}

		} else if (render_type == RENDER_TYPE::SINGLE_SPRITE) {
			if (!sp_proj.empty()) {

				constrain_at_barrel(sp_proj.at(0), campos);
				if (state.test(ProjectileState::destruction_initiated)) {
					constrain_at_destruction_point(sp_proj.at(0), campos);
				}

				//proj bounding box for debug
				box.setSize(bounding_box.dimensions);
				if (state.test(ProjectileState::destruction_initiated)) {
					box.setFillColor(sf::Color{ 255, 255, 60, 160 });
				} else {
					box.setFillColor(sf::Color{ 255, 255, 255, 160 });
				}
				box.setPosition(bounding_box.position.x - campos.x, bounding_box.position.y - campos.y);

				if (svc::globalBitFlagsLocator.get().test(svc::global_flags::greyblock_state)) {
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
		//destruction_point = point;
		if (completely) {
			state.set(ProjectileState::destroyed);
			return;
		}

		if (!state.test(ProjectileState::destruction_initiated)) {
			if(dir == FIRING_DIRECTION::UP || dir == FIRING_DIRECTION::LEFT) { destruction_point = bounding_box.position; }
			else { destruction_point = bounding_box.position + bounding_box.dimensions; }
			state.set(ProjectileState::destruction_initiated);
		}

		stats.damage = 0;

	}

	void Projectile::seed() {
		util::Random r{};
		stats.range += r.random_range(-stats.range_variance, stats.range_variance);
		float var = r.random_range_float(-stats.variance, stats.variance);
		switch (dir) {
		case FIRING_DIRECTION::LEFT:
			physics.velocity = { -stats.speed + (var / 2), var };
			physics.dir = components::DIRECTION::LEFT;
			break;
		case FIRING_DIRECTION::RIGHT:
			physics.velocity = { stats.speed + (var / 2), var };
			physics.dir = components::DIRECTION::RIGHT;
			break;
		case FIRING_DIRECTION::UP:
			physics.velocity = { var, -stats.speed + (var / 2) };
			physics.dir = components::DIRECTION::UP;
			break;
		case FIRING_DIRECTION::DOWN:
			physics.velocity = { var, stats.speed + (var / 2) };
			physics.dir = components::DIRECTION::DOWN;
			break;
		}
	}

	void Projectile::set_sprite() {


		for (int i = 0; i < num_sprites; ++i) {
			sp_proj.push_back(sf::Sprite());
		}

		for (auto& sprite : sp_proj) {

			set_orientation(sprite);

			if (type == WEAPON_TYPE::BRYNS_GUN) {
				sprite.setTexture(svc::assetLocator.get().t_bryns_gun_projectile);
				sprite.setTextureRect(sf::IntRect({ 0, 0 }, { 28, 12 }));
			}
			if (type == WEAPON_TYPE::PLASMER) {
				sprite.setTexture(svc::assetLocator.get().t_plasmer_projectile);
				sprite.setTextureRect(sf::IntRect({ 0, 0 }, { 38, 12 }));
			}
			if (type == WEAPON_TYPE::NOVA) {
				sprite.setTexture(svc::assetLocator.get().t_nova_projectile);
				sprite.setTextureRect(sf::IntRect({ 0, 0 }, { 28, 12 }));
			}
			if (type == WEAPON_TYPE::SKYCORPS_AR) {
				sprite.setTexture(svc::assetLocator.get().t_skycorps_ar_projectile);
				sprite.setTextureRect(sf::IntRect({ 0, 0 }, { 14, 14 }));
			}

			//sprite.setTexture(lookup::projectile_texture.at(WEAPON_TYPE::BRYNS_GUN));
			sprite.setPosition(physics.position);
		}
		//sp_proj = svc::assetLocator.get().sp_bryns_gun_projectile;
		//sp_proj = lookup::projectile_sprites.at(type);
		if (anim.num_sprites < 2) { sprite_id = 0; return; }
		util::Random r{};
		sprite_id = r.random_range(0, anim.num_sprites - 1);
	}

	void Projectile::set_orientation(sf::Sprite& sprite) {
		//assume right
		sprite.setScale({ 1.0f, 1.0f });
		sprite.setRotation(0.0f);
		sprite.setOrigin(0, 0);

		switch (dir) {

		case FIRING_DIRECTION::LEFT:
			sprite.scale({ -1.0f, 1.0f });
			break;
		case FIRING_DIRECTION::RIGHT:

			//do nothing
			break;
		case FIRING_DIRECTION::UP:
			sprite.rotate(-90);
			break;
		case FIRING_DIRECTION::DOWN:
			sprite.rotate(90);
			break;

		}

	}

	void Projectile::constrain_at_barrel(sf::Sprite& sprite, sf::Vector2<float>& campos) {
		if (dir == FIRING_DIRECTION::LEFT || dir == FIRING_DIRECTION::RIGHT) {
			if (abs(physics.position.x - fired_point.x) < max_dimensions.x) {
				int width = abs(physics.position.x - fired_point.x);
				sprite.setTextureRect(sf::IntRect({ (int)(max_dimensions.x - width), 0 }, { width, (int)max_dimensions.y }));
				bounding_box.dimensions.x = width;
			} else {
				sprite.setTextureRect(sf::IntRect({ 0, 0 }, { (int)(bounding_box.dimensions.x), (int)(bounding_box.dimensions.y) }));
				bounding_box.dimensions.x = max_dimensions.x;
			}
			if (dir == FIRING_DIRECTION::RIGHT) {
				sprite.setPosition(bounding_box.position.x - campos.x, bounding_box.position.y - campos.y);
			} else if (dir == FIRING_DIRECTION::LEFT) {
				sprite.setPosition(bounding_box.position.x + bounding_box.dimensions.x - campos.x, bounding_box.position.y - campos.y);
			}

		} else {
			bounding_box.dimensions.x = max_dimensions.y;
			if (abs(physics.position.y - fired_point.y) < max_dimensions.x) {
				int height = abs(physics.position.y - fired_point.y);
				sprite.setTextureRect(sf::IntRect({ (int)(max_dimensions.x - height), 0 }, { height, (int)max_dimensions.y }));
				bounding_box.dimensions.y = height;
			} else {
				sprite.setTextureRect(sf::IntRect({ 0, 0 }, { (int)(max_dimensions.x), (int)(max_dimensions.y) }));
				bounding_box.dimensions.y = max_dimensions.x;
			}
			if (dir == FIRING_DIRECTION::UP) {
				sprite.setPosition(bounding_box.position.x - campos.x, bounding_box.position.y + bounding_box.dimensions.y - campos.y);
			} else if (dir == FIRING_DIRECTION::DOWN) {
				sprite.setPosition(bounding_box.position.x + bounding_box.dimensions.x - campos.x, bounding_box.position.y - campos.y);
			}
		}
	}

	void Projectile::constrain_at_destruction_point(sf::Sprite& sprite, sf::Vector2<float>& campos) {
		if (dir == FIRING_DIRECTION::LEFT || dir == FIRING_DIRECTION::RIGHT) {
			if (dir == FIRING_DIRECTION::LEFT) {
				float rear = bounding_box.dimensions.x + physics.position.x;
				int width = abs(rear - destruction_point.x);
				sprite.setTextureRect(sf::IntRect({ 0, 0 }, { width, (int)max_dimensions.y }));
				bounding_box.dimensions.x = width;
				bounding_box.position.x = destruction_point.x;
				sprite.setPosition(bounding_box.position.x + bounding_box.dimensions.x - campos.x, bounding_box.position.y - campos.y);
				if (rear <= destruction_point.x) { destroy(true); }
			} else {
				float rear = physics.position.x - bounding_box.dimensions.x;
				int width = abs(rear - destruction_point.x);
				sprite.setTextureRect(sf::IntRect({ 0, 0 }, { width, (int)max_dimensions.y }));
				bounding_box.dimensions.x = width;
				bounding_box.position.x = destruction_point.x - width;
				sprite.setPosition(bounding_box.position.x - campos.x, bounding_box.position.y - campos.y);
				if (rear >= destruction_point.x) { destroy(true); }
			}

		} else {
			if (dir == FIRING_DIRECTION::UP) {
				float rear = bounding_box.dimensions.y + physics.position.y;
				int height = abs(rear - destruction_point.y);
				sprite.setTextureRect(sf::IntRect({ 0, 0 }, { height, (int)max_dimensions.y }));
				bounding_box.dimensions.y = height;
				bounding_box.position.y = destruction_point.y;
				sprite.setPosition(bounding_box.position.x - campos.x, bounding_box.position.y + bounding_box.dimensions.y - campos.y);
				if (rear <= destruction_point.y) { destroy(true); }
			} else {
				float rear = physics.position.y - bounding_box.dimensions.y;
				int height = abs(rear - destruction_point.y);
				sprite.setTextureRect(sf::IntRect({ 0, 0 }, { height, (int)max_dimensions.y }));
				bounding_box.dimensions.y = height;
				bounding_box.position.y = destruction_point.y - height;
				sprite.setPosition(bounding_box.position.x + bounding_box.dimensions.x - campos.x, bounding_box.position.y - campos.y);
				if (rear >= destruction_point.y) { destroy(true); }
			}
		}
	}


} // end arms

/* Projectile_cpp */
