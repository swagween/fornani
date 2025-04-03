
#pragma once

#include "fornani/entities/animation/AnimatedSprite.hpp"
#include "fornani/entities/player/Wardrobe.hpp"
#include "fornani/gui/Gizmo.hpp"
#include "fornani/gui/InventorySelector.hpp"
#include "fornani/gui/gizmos/DescriptionGizmo.hpp"

namespace fornani::gui {

struct Slider {
	FreeConstituent body{};
	int selection{};
};

class OutfitterGizmo : public Gizmo {
  public:
	OutfitterGizmo(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f placement);
	void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) override;
	void render(automa::ServiceProvider& svc, WindowManager& window, [[maybe_unused]] player::Player& player, sf::Vector2f cam, bool foreground = false) override;
	bool handle_inputs(config::ControllerMap& controller, [[maybe_unused]] audio::Soundboard& soundboard) override;
	[[nodiscard]] auto get_outfit() const -> std::array<int, static_cast<int>(player::ApparelType::END)> { return m_outfit; }
	[[nodiscard]] auto has_changed() const -> bool { return m_change_outfit; }

	void close();

  private:
	std::unique_ptr<DescriptionGizmo> m_description;
	void init_sliders();
	void update_sliders(player::Player& player);
	void debug();
	int m_max_slots{};
	int wardrobe_index;
	int m_current_item_id{};
	sf::Sprite m_sprite;
	sf::Sprite m_apparel_sprite;
	std::array<Slider, static_cast<int>(player::ApparelType::END)> m_sliders;
	InventorySelector m_selector;
	Constituent m_row;
	util::RectPath m_path;
	anim::AnimatedSprite m_wires;
	sf::Vector2f m_grid_offset{};
	std::array<int, static_cast<int>(player::ApparelType::END)> m_outfit{};
	sf::Vector2i m_outfit_delta{};
	bool m_init{};
	bool m_change_outfit{};
};

} // namespace fornani::gui
