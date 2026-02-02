
#include <fornani/automa/GameplayState.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::automa {

GameplayState::GameplayState(ServiceProvider& svc, player::Player& player, std::string_view scene, int room_number) : GameState(svc, player, scene, room_number), p_services{&svc} {
	svc.controller_map.set_action_set(config::ActionSet::Platformer);
	svc.events.play_song_event.attach_to(p_slot, &GameplayState::play_song_by_id, this);
}

void GameplayState::play_song_by_id(int id) { p_services->music_player.play_song_by_id(p_services->finder, id); }

} // namespace fornani::automa
