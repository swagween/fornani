
#include "fornani/world/SwitchBlock.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/particle/Effect.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::world {

SwitchBlock::SwitchBlock(automa::ServiceProvider& svc, Map& map, sf::Vector2f position, int button_id, int type)
	: Animatable(svc, "switch_blocks", constants::i_resolution_vec), m_type(static_cast<SwitchType>(type)), m_button_id(button_id), m_collider{map, {constants::f_cell_vec}} {
	get_collider().physics.position = position;
	m_state = SwitchBlockState::full;
	m_previous_state = SwitchBlockState::full;
	if (svc.data.switch_is_activated(button_id)) { m_state = SwitchBlockState::empty; }
	set_channel(static_cast<int>(m_type));
	m_collider.get()->set_attribute(shape::ColliderAttributes::fixed);
}

void SwitchBlock::update(automa::ServiceProvider& svc, Map& map, player::Player& player) {
	tick();
	m_collider.get()->set_flag(shape::ColliderFlags::intangible, m_state == SwitchBlockState::empty);
	set_parameters({static_cast<int>(m_state), 1, 1, -1});
	if (m_state != SwitchBlockState::empty) { handle_collision(player.collider); }
	for (auto& enemy : map.enemy_catalog.enemies) { handle_collision(enemy->get_collider()); }
	if (switched()) {
		map.effects.push_back(entity::Effect(svc, "small_explosion", get_collider().get_center()));
		svc.soundboard.flags.world.set(audio::World::block_toggle);
	}
	m_previous_state = m_state;
}

void SwitchBlock::handle_collision(shape::Collider& other) const { other.handle_collider_collision(*m_collider.get()); }

void SwitchBlock::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam, bool background) {
	if (m_state == SwitchBlockState::empty && !background) { return; }
	set_position(get_collider().physics.position - cam);

	svc.greyblock_mode() ? get_collider().render(win, cam) : win.draw(*this);
}

void SwitchBlock::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) const {
	if (proj.transcendent()) { return; }
	if (m_state == SwitchBlockState::empty) { return; }
	if (proj.get_collider().collides_with(m_collider.get()->bounding_box)) {
		if (!proj.destruction_initiated()) {
			map.effects.push_back(entity::Effect(svc, "inv_hit", proj.get_position()));
			svc.soundboard.flags.world.set(audio::World::hard_hit);
		}
		proj.destroy(false);
	}
}

} // namespace fornani::world
