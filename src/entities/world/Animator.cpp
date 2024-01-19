//
//  Animator.cpp
//  entity
//
//

#include "Animator.hpp"
#include "../../setup/ServiceLocator.hpp"

namespace entity {
	void Animator::update() {

		large = scaled_dimensions.x == 2;
		if (large) {
			sprite.setTexture(svc::assetLocator.get().t_large_animators);
			sprite_dimensions = { 64, 64 };
		} else {
			sprite.setTexture(svc::assetLocator.get().t_small_animators);
			sprite_dimensions = { 32, 32 };
		}
		position = static_cast<Vec>(scaled_position * A_UNIT_SIZE);
		Vec adjusted_pos = Vec(scaled_position.x * entity::A_UNIT_SIZE + entity::large_animator_offset.x, scaled_position.y * entity::A_UNIT_SIZE + entity::large_animator_offset.y);
		dimensions = static_cast<Vec>(scaled_dimensions * A_UNIT_SIZE);
		bounding_box.set_position(adjusted_pos);
		int converted_id = large ? id - 100 : id - 200;

		anim.update();
		anim.current_state.update();
		if(activated) {
			
		}
		//get UV coords
		int u = converted_id * scaled_dimensions.x * A_UNIT_SIZE;
		int v = get_frame() * dimensions.y;
		sprite.setTextureRect(sf::IntRect({ u, v }, { sprite_dimensions.x, sprite_dimensions.y }));
	}
	void Animator::render(sf::RenderWindow& win, Vec campos) {
		sprite.setPosition((int)(position.x - campos.x), (int)(position.y - campos.y));
		win.draw(sprite);
		svc::counterLocator.get().at(svc::draw_calls)++;
		sf::RectangleShape box{};
		if (activated) {
			box.setFillColor(sf::Color{ 80, 180, 120, 100 });
		}
		else {
			box.setFillColor(sf::Color{ 180, 120, 80, 100 });
		}
		box.setOutlineColor(sf::Color::White);
		box.setOutlineThickness(-1);
		box.setPosition(bounding_box.position - campos);
		box.setSize(bounding_box.dimensions);
		/*win.draw(box);
		svc::counterLocator.get().at(svc::draw_calls)++;*/
	}
	int Animator::get_frame() {
		return anim.get_frame();
	}
} // end entity

/* Animator_cpp */
