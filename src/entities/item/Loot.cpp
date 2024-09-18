
#include "Loot.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../player/Player.hpp"


namespace item {
Loot::Loot(automa::ServiceProvider& svc, sf::Vector2<int> drop_range, float probability, sf::Vector2<float> pos, int delay_time, bool special) {

	auto drop_rate = svc.random.random_range(drop_range.x, drop_range.y);
	position = pos;

	std::string_view key{};
	for (int i = 0; i < drop_rate; ++i) {
		if (svc.random.percent_chance(50.f) && special) {
			key = "gem";
		} else if (svc.random.percent_chance(8) && !flags.test(LootState::heart_dropped)) {
			key = "heart";
			flags.set(LootState::heart_dropped);
		} else {
			key = "orb";
		}
		float randx = svc.random.random_range_float(-100.0f, 100.0f);
		float randy = svc.random.random_range_float(-100.0f, 100.0f);
		drops.push_back(Drop(svc, key, probability, delay_time));
		drops.back().set_position(pos);
		drops.back().get_collider().physics.apply_force({randx, randy});
	}
}

void Loot::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	std::erase_if(drops, [](auto const& d) { return d.is_completely_gone(); });
	for (auto& drop : drops) {
		drop.update(svc, map);
		if (drop.get_collider().bounding_box.overlaps(player.collider.bounding_box) && !drop.is_inactive() && !drop.is_completely_gone() && drop.delay_over()) {
			player.give_drop(drop.get_type(), static_cast<float>(drop.get_value()));
			if (drop.get_type() == DropType::heart) {
				svc.soundboard.flags.item.set(audio::Item::heal);
			} else if(drop.get_rarity() == common) {
				svc.soundboard.flags.item.set(audio::Item::orb_low);
			} else if (drop.get_rarity() == uncommon) {
				svc.soundboard.flags.item.set(audio::Item::orb_medium);
			} else if (drop.get_rarity() == rare) {
				svc.soundboard.flags.item.set(audio::Item::orb_high);
			} else if (drop.get_rarity() == priceless) {
				svc.soundboard.flags.item.set(audio::Item::orb_max);
			}
			drop.deactivate();
		}
	}
}

void Loot::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> campos) {
	for (auto& drop : drops) { drop.render(svc, win, campos); }
}

void Loot::set_position(sf::Vector2<float> pos) { position = pos; }

} // namespace item
