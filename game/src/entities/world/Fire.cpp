
#include "fornani/entities/world/Fire.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::world {

Fire::Fire(automa::ServiceProvider& svc, sf::Vector2f position, int lookup)
	: Animatable(svc, "fire", {20, 20}), size(lookup == 244 ? 2 : 1), bounding_box{{32.f, 32.f}}, sprite_offset{-4.f, -8.f}, sparkler(svc, {32.f, 32.f}, sf::Color::White, "fire") {
	bounding_box.set_position(position);
	set_parameters({0, 5, 18, -1});
	sparkler.set_position(bounding_box.get_position());
	if (size == 2) {
		set_texture(svc.assets.get_texture("bonfire"));
		set_dimensions({36, 43});
		sprite_offset = {-12.f, -38.f};
		bounding_box.set_dimensions({48.f, 48.f});
	}
}

void Fire::update(automa::ServiceProvider& svc, player::Player& player, Map& map, std::optional<std::unique_ptr<gui::Console>>& console) {
	tick();
	sparkler.update(svc);
	if (svc.ticker.every_x_ticks(64)) {
		auto direction = Direction{};
		direction.und = UND::up;
		auto offset = sf::Vector2f{0.f, 48.f};
		map.active_emitters.push_back(vfx::Emitter(svc, bounding_box.get_position() - offset, bounding_box.get_dimensions(), "smoke", sf::Color::Transparent, direction));
	}
}

void Fire::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	set_position(bounding_box.get_position() + sprite_offset - cam);
	win.draw(*this);
	sparkler.render(win, cam);
	if (svc.greyblock_mode()) {}
}

} // namespace fornani::world
