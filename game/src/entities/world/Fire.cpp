
#include "fornani/entities/world/Fire.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::world {

Fire::Fire(automa::ServiceProvider& svc, sf::Vector2<float> position, int lookup)
	: size(lookup == 244 ? 2 : 1), bounding_box{{48.f, 48.f}}, sprite_offset{-12.f, -38.f}, sparkler(svc, {48.f, 48.f}, sf::Color::White, "fire"), sprite(svc.assets.get_texture("fire"), {72, 86}),
	  inspectable(svc, sf::Vector2<std::uint32_t>{48, 48}, sf::Vector2<std::uint32_t>{static_cast<std::uint32_t>(position.x), static_cast<std::uint32_t>(position.y)}) {
	auto bb_offset = util::constants::f_cell_vec - bounding_box.get_dimensions();
	bounding_box.set_position(position + sf::Vector2<float>{bb_offset.x * 0.5f, bb_offset.y});
	sprite.push_params("basic", {0, 5, 18, -1});
	sprite.set_params("basic");
	sparkler.set_position(bounding_box.get_position());
	inspectable.position = bounding_box.get_position();
}

void Fire::update(automa::ServiceProvider& svc, player::Player& player, Map& map, std::optional<std::unique_ptr<gui::Console>>& console, dj::Json& set) {
	inspectable.update(svc, player, console, set);
	sparkler.update(svc);
	sprite.update(bounding_box.get_position() + sprite_offset);
	if (svc.ticker.every_x_ticks(64)) {
		auto direction = dir::Direction{};
		direction.und = dir::UND::up;
		auto offset = sf::Vector2<float>{0.f, 48.f};
		map.active_emitters.push_back(vfx::Emitter(svc, bounding_box.get_position() - offset, bounding_box.get_dimensions(), "smoke", sf::Color::Transparent, direction));
	}
}

void Fire::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	sprite.render(svc, win, cam);
	sparkler.render(svc, win, cam);
	inspectable.render(svc, win, cam);
	if (svc.greyblock_mode()) {}
}

} // namespace fornani::world
