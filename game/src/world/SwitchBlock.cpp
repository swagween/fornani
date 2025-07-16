
#include "fornani/world/SwitchBlock.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/particle/Effect.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::world {

SwitchBlock::SwitchBlock(automa::ServiceProvider& svc, sf::Vector2f position, int button_id, int type) : Animatable(svc, "switch_blocks", constants::i_resolution_vec), m_type(static_cast<SwitchType>(type)), m_button_id(button_id) {
	collider = shape::Collider(constants::f_cell_vec);
	collider.physics.position = position;
	collider.sync_components();
	if (svc.data.switch_is_activated(button_id)) { m_state = SwitchBlockState::empty; }
	set_channel(static_cast<int>(m_type));
}

void SwitchBlock::update(automa::ServiceProvider& svc, Map& map, player::Player& player) {
	tick();
	set_parameters({static_cast<int>(m_state), 1, 1, -1});
	if (m_state != SwitchBlockState::empty) { handle_collision(player.collider); }
	if (switched()) {
		map.effects.push_back(entity::Effect(svc, "small_explosion", collider.physics.position));
		svc.soundboard.flags.world.set(audio::World::block_toggle);
	}
	m_previous_state = m_state;
}

void SwitchBlock::handle_collision(shape::Collider& other) const { other.handle_collider_collision(collider); }

void SwitchBlock::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam, bool background) {
	if (m_state == SwitchBlockState::empty && !background) { return; }
	set_position(collider.physics.position - cam);

	svc.greyblock_mode() ? collider.render(win, cam) : win.draw(*this);
}

void SwitchBlock::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) const {
	if (proj.transcendent()) { return; }
	if (m_state == SwitchBlockState::empty) { return; }
	if (proj.get_bounding_box().overlaps(collider.bounding_box)) {
		if (!proj.destruction_initiated()) {
			map.effects.push_back(entity::Effect(svc, "inv_hit", proj.get_position()));
			svc.soundboard.flags.world.set(audio::World::hard_hit);
		}
		proj.destroy(false);
	}
}

} // namespace fornani::world
