//
//  Antenna.cpp
//  entity
//
//

#include "Antenna.hpp"
#include "../../setup/ServiceLocator.hpp"

namespace entity {

	Antenna::Antenna(Vec pos, sf::Color col, float agf) : scaled_position(pos), color(col), antenna_gravity_force(agf) {

		collider = shape::Collider(sf::Vector2<float>{4.f, 4.f}, sf::Vector2<float>{ pos.x, pos.x});
		collider.bounding_box.dimensions = Vec(4, 4);
		collider.physics.position = static_cast<Vec>(pos) * lookup::unit_size_f;
		collider.bounding_box = shape::Shape(collider.bounding_box.dimensions);
		collider.bounding_box.set_position(static_cast<Vec>(pos));
		box.setSize(collider.bounding_box.dimensions);
		box.setFillColor(color);
		collider.left_detector.dimensions = sf::Vector2<float>(1.f, 4.f);
		collider.right_detector.dimensions = sf::Vector2<float>(1.f, 4.f);

	}

	void Antenna::update() {

		collider.physics.update_dampen();
		collider.sync_components();

	}

	void Antenna::set_target_position(Vec new_position) {

		float gx = collider.physics.position.x;
		float gy = collider.physics.position.y;
		float mx = new_position.x - collider.bounding_box.dimensions.x / 2;
		float my = new_position.y - collider.bounding_box.dimensions.y / 2;

		float force_x = mx - gx;
		float force_y = my - gy;
		float mag = sqrt((force_x * force_x) + (force_y * force_y));
		float str = antenna_gravity_force / mag * mag;
		force_x *= str;
		force_y *= str;
		collider.physics.apply_force({ force_x, force_y });

	}

	void Antenna::render(sf::RenderWindow& win, Vec campos) {

		box.setPosition((int)(collider.bounding_box.position.x - campos.x), (int)(collider.bounding_box.position.y - campos.y));
		//win.draw(box);
		svc::counterLocator.get().at(svc::draw_calls)++;

		collider.render(win, campos);

	}
} // end entity

/* Antenna_cpp */
