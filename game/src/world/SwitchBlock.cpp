#include "fornani/world/SwitchBlock.hpp"
#include <cmath>
#include "fornani/entities/player/Player.hpp"
#include "fornani/particle/Effect.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"
#include "fornani/world/SwitchBlock.hpp"

namespace fornani::world {

SwitchBlock::SwitchBlock(automa::ServiceProvider& svc, sf::Vector2f position, int button_id, int type) : type(static_cast<SwitchType>(type)), button_id(button_id), sprite{svc.assets.get_texture("switch_blocks")} {
	collider = shape::Collider({32.f, 32.f});
	collider.physics.position = position;
	collider.sync_components();
	sprite.setTextureRect(sf::IntRect{{static_cast<int>(type) * 32, static_cast<int>(state) * 32}, {32, 32}});
	if (svc.data.switch_is_activated(button_id)) { state = SwitchBlockState::empty; }
}

void SwitchBlock::update(automa::ServiceProvider& svc, Map& map, player::Player& player) {
	if (state != SwitchBlockState::empty) { handle_collision(player.collider); }
	if (switched()) {
		map.effects.push_back(entity::Effect(svc, "small_explosion", collider.physics.position, {}, 0, 0));
		svc.soundboard.flags.world.set(audio::World::block_toggle);
	}
	previous_state = state;
}

void SwitchBlock::handle_collision(shape::Collider& other) const { other.handle_collider_collision(collider); }

void SwitchBlock::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam, bool background) {
	if (state == SwitchBlockState::empty && !background) { return; }
	sprite.setPosition(collider.physics.position - cam);
	sprite.setTextureRect(sf::IntRect{{static_cast<int>(type) * 32, static_cast<int>(state) * 32}, {32, 32}});
	if (svc.greyblock_mode()) {
		collider.render(win, cam);
	} else {
		win.draw(sprite);
	}
}

void SwitchBlock::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj, int power) {
	if (proj.transcendent()) { return; }
	if (state == SwitchBlockState::empty) { return; }
	if (proj.get_bounding_box().overlaps(collider.bounding_box)) {
		if (!proj.destruction_initiated()) {
			map.effects.push_back(entity::Effect(svc, "inv_hit", proj.get_position(), {}, 0, 6));
			svc.soundboard.flags.world.set(audio::World::hard_hit);
		}
		proj.destroy(false);
	}
}

} // namespace fornani::world
