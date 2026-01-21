
#include <fornani/core/Common.hpp>
#include <fornani/entities/item/Loot.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>

namespace fornani::item {

Loot::Loot(automa::ServiceProvider& svc, world::Map& map, player::Player& player, sf::Vector2<int> drop_range, float probability, sf::Vector2f pos, int delay_time, bool special, int special_id, int individual_delay) {

	auto const drop_rate = random::random_range(drop_range.x, drop_range.y);
	position = pos;

	std::string_view key{};
	for (int i = 0; i < drop_rate; ++i) {
		if (random::percent_chance(gem_chance_v) && special) {
			key = "gems";
		} else if (random::percent_chance(heart_chance_v) && !flags.test(LootState::heart_dropped)) {
			key = "hearts";
			flags.set(LootState::heart_dropped);
		} else {
			key = "orbs";
		}
		auto xinit = svc.data.drop[key]["initial_velocity"][0].as<float>();
		auto yinit = svc.data.drop[key]["initial_velocity"][1].as<float>();
		auto rand_vec = random::random_vector_float({-xinit, -yinit}, {xinit, 0});
		drops.push_back(std::make_unique<Drop>(svc, map, key, probability, delay_time, special_id));
		drops.back()->set_position(pos);
		if (player.has_item_equipped(svc.data.item_id_from_label("magnet"))) { rand_vec *= 0.01f; }
		auto delayed = individual_delay > 0;
		drops.back()->apply_force(rand_vec, delayed);
		if (delayed) { drops.back()->set_delay(individual_delay * i); }
	}
}

void Loot::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	std::erase_if(drops, [](auto const& d) { return d->is_completely_gone(); });
	for (auto& drop : drops) { drop->update(svc, map, player); }
}

void Loot::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f campos) {
	for (auto& drop : drops) { drop->render(svc, win, campos); }
}

void Loot::set_position(sf::Vector2f pos) { position = pos; }

} // namespace fornani::item
