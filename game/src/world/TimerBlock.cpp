
#include "fornani/world/TimerBlock.hpp"
#include <fornani/utils/Constants.hpp>
#include "fornani/entities/player/Player.hpp"
#include "fornani/particle/Effect.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::world {

TimerBlock::TimerBlock(automa::ServiceProvider& svc, sf::Vector2<std::uint32_t> pos, TimerBlockType type, int id)
	: Drawable(svc, "timer_blocks"), IWorldPositionable(pos, constants::u32_cell_vec), m_bounds(constants::f_cell_vec), m_type{type}, m_id{id} {
	m_bounds.set_position(get_world_position());
}

void TimerBlock::update(automa::ServiceProvider& svc, Map& map, player::Player& player) {
	if (player.get_collider().hurtbox.overlaps(m_bounds)) { m_flags.set(TimerBlockFlags::reached); }
	if (m_flags.test(TimerBlockFlags::reached)) {
		switch (m_type) {
		case TimerBlockType::start:
			if (!m_flags.test(TimerBlockFlags::triggered)) {
				svc.world_timer.start();
				m_flags.set(TimerBlockFlags::triggered);
				for (auto& block : map.timer_blocks) {
					if (block.get_id() == m_id && block.get_type() == m_type) { block.trigger(); }
				}
			}
			break;
		case TimerBlockType::finish:
			if (!m_flags.test(TimerBlockFlags::triggered)) {
				svc.world_timer.finish(svc);
				m_flags.set(TimerBlockFlags::triggered);
				for (auto& block : map.timer_blocks) {
					if (block.get_id() == m_id && block.get_type() == m_type) { block.trigger(); }
				}
			}
			break;
		}
	}
}

void TimerBlock::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	set_position(get_world_position() - cam);
	m_bounds.render(win, cam);
	win.draw(*this);
}

} // namespace fornani::world
