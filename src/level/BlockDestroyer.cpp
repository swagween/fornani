#include "BlockDestroyer.hpp"
#include "../service/ServiceProvider.hpp"
#include "../level/Map.hpp"
#include "../particle/Effect.hpp"

namespace world {

BlockDestroyer::BlockDestroyer(sf::Vector2<int> pos, int quest_id) : position(pos), quest_id(quest_id) {}

void BlockDestroyer::update(automa::ServiceProvider& svc, Map& map) {
	if (svc.quest.get_progression(fornani::QuestType::destroyers, quest_id) > 0) { flags.set(DestroyerState::detonated); }
	if (flags.test(DestroyerState::detonated)) {
		svc.data.get_layers(map.room_id).at(MIDDLEGROUND).grid.destroy_cell(position);
		map.generate_layer_textures(svc);
		map.effects.push_back(entity::Effect(svc, static_cast<sf::Vector2<float>>(position) * svc.constants.cell_size, {}, 0, 0));
		svc.soundboard.flags.world.set(audio::World::breakable_shatter);
	}
}

} // namespace world
