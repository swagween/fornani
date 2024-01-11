//
//  Animator.cpp
//  entity
//
//

#include "Animator.hpp"
#include "../../setup/ServiceLocator.hpp"

namespace entity {
	void Animator::update() {
		sprite.setTexture(svc::assetLocator.get().t_large_animators);
		position = static_cast<Vec>(scaled_position * A_UNIT_SIZE);
		dimensions = static_cast<Vec>(scaled_dimensions * A_UNIT_SIZE);
		bounding_box.set_position(position);

		bool large = scaled_dimensions.x == 2;
		int converted_id = large ? id - 100 : id - 200;

		anim.update();
		anim.current_state.update();
		if(activated) {
			
		}
		//get UV coords
		int u = converted_id * 2 * A_UNIT_SIZE;
		int v = get_frame() * dimensions.y;
		sprite.setTextureRect(sf::IntRect({ u, v }, { sprite_dimensions.x, sprite_dimensions.y }));
	}
	void Animator::render(sf::RenderWindow& win, Vec campos) {
		sprite.setPosition(bounding_box.position.x - campos.x, bounding_box.position.y - campos.y);
		win.draw(sprite);
	}
	int Animator::get_frame() {
		return anim.get_frame();
	}
} // end entity

/* Animator_cpp */
