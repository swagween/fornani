
#include "Fire.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../player/Player.hpp"
#include "../../level/Map.hpp"

namespace world {

Fire::Fire(automa::ServiceProvider& svc, sf::Vector2<float> position, int lookup)
	: size(lookup == 244 ? 2 : 1), bounding_box{{48.f, 48.f}}, sprite_offset{-12.f, -38.f}, sparkler(svc, {48.f, 48.f}, sf::Color::White, "fire"), sprite(svc.assets.t_fire, {72, 86}), inspectable(svc, sf::Vector2<uint32_t>{48, 48}, sf::Vector2<uint32_t>{static_cast<uint32_t>(position.x), static_cast<uint32_t>(position.y)}) {
	auto bb_offset =  svc.constants.f_cell_vec - bounding_box.dimensions;
	bounding_box.set_position(position + sf::Vector2<float>{bb_offset.x * 0.5f, bb_offset.y});
	sprite.push_params("basic", {0, 5, 18, -1});
	sprite.set_params("basic");
	sparkler.set_position(bounding_box.position);
	inspectable.position = bounding_box.position;
}

void Fire::update(automa::ServiceProvider& svc, player::Player& player, Map& map, gui::Console& console, dj::Json& set) {
	inspectable.update(svc, player, console, set);
	sparkler.update(svc);
	sprite.update(bounding_box.position + sprite_offset);
	if (svc.ticker.every_x_ticks(64)) {
		auto direction = dir::Direction{};
		direction.und = dir::UND::up;
		auto offset = sf::Vector2<float>{0.f, 48.f};
		map.active_emitters.push_back(vfx::Emitter(svc, bounding_box.position - offset, bounding_box.dimensions, "smoke", sf::Color::Transparent, direction));
	}
}

void Fire::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	sprite.render(svc, win, cam);
	sparkler.render(svc, win, cam);
	inspectable.render(svc, win, cam);
	if (svc.greyblock_mode()) { bounding_box.render(win, cam); }
}

} // namespace world