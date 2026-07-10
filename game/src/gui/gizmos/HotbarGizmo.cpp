
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/gizmos/HotbarGizmo.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::gui {

HotbarGizmo::HotbarGizmo(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f placement)
	: Gizmo("Hotbar", false), m_sprite{svc, "hotbar_gizmo"}, m_bounds{svc.finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, "hotbar", 128, util::InterpolationType::cubic}, m_screen_sprite{svc, "pioneer_screen"},
	  m_gun_display{svc, "heads_up_display_gun", {44, 9}} {
	m_placement = placement;
	m_bounds.set_section("start");

	auto u_dimensions = sf::Vector2u{24, 24};
	if (!m_screen.resize(u_dimensions)) { NANI_LOG_WARN(m_logger, "Failed to resize map texture"); }
	m_screen.draw(m_screen_sprite.get_sprite());
	m_screen.display();
	m_screen.setRepeated(true);

	m_gun_display.set_channel(2);
	m_outline_offset = sf::Vector2f{18.f, 24.f};
}

void HotbarGizmo::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) {
	Gizmo::update(svc, player, map, position);

	m_physics.position = m_placement + m_bounds.get_position() + position;
	m_bounds.update();
	auto minimum_height = 40.f;
	auto dim = player.hotbar ? sf::Vector2f{0.f, minimum_height + 30.f * std::max(static_cast<int>(player.hotbar->size() - 1), 0)} : sf::Vector2f{0.f, minimum_height};
	m_bounds.set_dimensions(dim);
}

void HotbarGizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, LightShader& shader, Palette& palette, sf::Vector2f cam, bool foreground) {
	if (is_foreground() != foreground) { return; }

	auto sz = player.hotbar ? sf::Vector2i{96, 60 + std::max(static_cast<int>(player.hotbar->size() - 1), 0) * 30} : sf::Vector2i{96, 60};
	auto screen = sf::Sprite{m_screen.getTexture()};
	screen.setTextureRect(sf::IntRect{{}, sz});
	screen.setPosition(m_physics.position + sf::Vector2f{8.f, 16.f} - cam);
	win.draw(screen);

	// draw guns
	if (player.hotbar) {
		auto gun_offset = m_outline_offset + sf::Vector2f{-12.f, 6.f};
		auto gun_dim = sf::Vector2f{0.f, 30.f};
		for (auto [i, gun] : std::views::enumerate(player.hotbar->get_tags())) {
			auto yoff = gun_dim * static_cast<float>(i);
			auto id = svc.data.get_gun_id_from_tag(gun);
			m_gun_display.set_position(m_physics.position + gun_offset + yoff - cam);
			m_gun_display.set_frame(id);
			win.draw(m_gun_display);
			if (id == m_hovered_gun) {
				m_sprite.set_texture_rect({{0, 42}, {40, 15}});
				m_sprite.set_position(m_physics.position + m_outline_offset + yoff - cam);
				win.draw(m_sprite);
			}
		}
	}

	m_sprite.set_texture_rect({{}, {56, 22}});
	m_sprite.set_position(m_physics.position - cam);
	shader.submit(win, palette, m_sprite.get_sprite());

	m_sprite.set_texture_rect({{0, 22}, {56, 20}});
	m_sprite.set_position(m_physics.position + m_bounds.get_dimensions() - cam);
	shader.submit(win, palette, m_sprite.get_sprite());
}

} // namespace fornani::gui
