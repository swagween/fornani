
#include "fornani/entities/world/Fire.hpp"
#include <fornani/graphics/Renderer.hpp>
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
	if (svc.ticker.every_x_ticks(64)) { map.spawn_emitter(svc, "smoke", bounding_box.get_center() - sf::Vector2f{0.f, 32.f}, {UND::up}, bounding_box.get_dimensions()); }
}

void Fire::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (!debug::is_production()) { return; }
	set_position(bounding_box.get_position() + sprite_offset - cam);
	win.draw(*this);
}

void Fire::submit(Renderer& renderer) {
	auto const pos = bounding_box.get_position() + sprite_offset;
	auto const& frame = get_sprite().getTextureRect();

	sf::FloatRect dest{pos, sf::Vector2f{frame.size}};
	renderer.submit(get_sprite().getTexture(), dest, frame, RenderLayer::background_entities);
	sparkler.submit(renderer);
}

} // namespace fornani::world
