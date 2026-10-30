
#include <fornani/entities/enemy/catalog/Geyser.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

Geyser::Geyser(automa::ServiceProvider& svc, world::Map& map, int variant) : Enemy(svc, map, "geyser"), m_services{&svc}, m_variant{static_cast<GeyserVariant>(variant)} {
	flags.state.set(StateFlags::intangible);
	flags.state.set(StateFlags::invisible);
}

void Geyser::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (died()) { return; }

	if (svc.ticker.every_x_ticks(80)) {
		m_bubbles.push_back(ToxicBubble{.lifetime{random::range(300, 500)}});
		auto& bubble = m_bubbles.back();
		bubble.attack.hit.bounds.setRadius(40.f);
		bubble.attack.hit.activate();
		bubble.lifetime.start();
		bubble.physics.position = get_collider().get_center();
	}

	if (svc.ticker.every_x_ticks(40)) {
		auto const tag = random::percent_chance(50) ? "small_gas_bubble" : "gas_bubble";
		map.spawn_effect(svc, tag, get_collider().get_center(), sf::Vector2f{random::range_float(-0.1f, 0.1f), random::range_float(-1.8f, -1.2f)}, static_cast<int>(m_variant));
	}

	for (auto& bubble : m_bubbles) {
		bubble.physics.velocity.y = -0.5f;
		bubble.physics.simple_update();
		bubble.attack.update();
		if (!player.has_item_equipped("gas_mask")) { bubble.attack.poison_player(player, 0.008f); }
		bubble.attack.set_position(bubble.physics.position);
		bubble.lifetime.update();
	}

	std::erase_if(m_bubbles, [](auto const& b) { return b.lifetime.is_almost_complete(); });
}

void Geyser::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	// for (auto& bubble : m_bubbles) { bubble.attack.render(win, cam); }
}

} // namespace fornani::enemy
