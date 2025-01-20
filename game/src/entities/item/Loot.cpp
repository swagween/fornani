
#include "fornani/entities/item/Loot.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/entities/player/Player.hpp"


namespace item {
Loot::Loot(automa::ServiceProvider& svc, sf::Vector2<int> drop_range, float probability, sf::Vector2<float> pos, int delay_time, bool special, int special_id) {

	auto drop_rate = svc.random.random_range(drop_range.x, drop_range.y);
	position = pos;

	std::string_view key{};
	for (int i = 0; i < drop_rate; ++i) {
		if (svc.random.percent_chance(0.08f) && special) {
			key = "gem";
		} else if (svc.random.percent_chance(8) && !flags.test(LootState::heart_dropped)) {
			key = "heart";
			flags.set(LootState::heart_dropped);
		} else {
			key = "orb";
		}
		float randx = svc.random.random_range_float(-40.0f, 40.0f);
		float randy = svc.random.random_range_float(-40.0f, 40.0f);
		drops.push_back(std::make_unique<Drop>(svc, key, probability, delay_time, special_id));
		drops.back()->set_position(pos);
		drops.back()->apply_force({randx, randy});
	}
}

void Loot::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	std::erase_if(drops, [](auto const& d) { return d->is_completely_gone(); });
	for (auto& drop : drops) {
		drop->update(svc, map);
		if (drop->collides_with(player.collider.bounding_box) && !drop->is_inactive() && !drop->is_completely_gone() && drop->delay_over()) {
			player.give_drop(drop->get_type(), static_cast<float>(drop->get_value()));
			if (drop->get_type() == DropType::gem) {
				svc.soundboard.flags.item.set(audio::Item::gem);
			} else if (drop->get_type() == DropType::heart) {
				svc.soundboard.flags.item.set(audio::Item::heal);
			} else if(drop->get_rarity() == common) {
				svc.soundboard.flags.item.set(audio::Item::orb_low);
			} else if (drop->get_rarity() == uncommon) {
				svc.soundboard.flags.item.set(audio::Item::orb_medium);
			} else if (drop->get_rarity() == rare) {
				svc.soundboard.flags.item.set(audio::Item::orb_high);
			} else if (drop->get_rarity() == priceless) {
				svc.soundboard.flags.item.set(audio::Item::orb_max);
			}
			drop->deactivate();
		}
	}
}

void Loot::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> campos) {
	for (auto& drop : drops) { drop->render(svc, win, campos); }
}

void Loot::set_position(sf::Vector2<float> pos) { position = pos; }

} // namespace item
