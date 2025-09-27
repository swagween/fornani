
#include <fornani/entities/Mobile.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani {

Mobile::Mobile(automa::ServiceProvider& svc, std::string_view label, sf::Vector2i dimensions) : Animatable(svc, label, dimensions) {}

void Mobile::face_player(player::Player& player) { directions.desired.set((player.collider.get_center().x < collider.get_center().x) ? LNR::left : LNR::right); }

bool Mobile::player_behind(player::Player& player) const { return player.collider.physics.position.x + player.collider.bounding_box.get_dimensions().x * 0.5f < collider.physics.position.x + collider.dimensions.x * 0.5f; }

void Mobile::post_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (p_flags.consume(MobileState::flip)) {
		if (directions.desired.lnr != directions.actual.lnr) { flip(); }
		directions.desired.unlock();
		directions.actual = directions.desired;
	}
	Animatable::tick();
}

} // namespace fornani
