
#pragma once

#include <fornani/gui/Gizmo.hpp>
#include <fornani/gui/InventorySelector.hpp>
#include <fornani/gui/console/TextWriter.hpp>
#include <fornani/utils/RectPath.hpp>

namespace fornani::gui {

struct QuestEntry {
	std::string tag;
	sf::Text title;
	sf::Vector2f offset{};
};

enum class JournalSection : std::uint8_t { quest, bestiary, END };

class JournalGizmo : public Gizmo {
  public:
	JournalGizmo(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f placement);
	void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, LightShader& shader, Palette& palette, sf::Vector2f cam, bool foreground = false) override;
	bool handle_inputs(input::InputSystem& controller, [[maybe_unused]] audio::Soundboard& soundboard) override;

	[[nodiscard]] auto is_quest() const -> bool { return m_section == JournalSection::quest; }
	[[nodiscard]] auto is_bestiary() const -> bool { return m_section == JournalSection::bestiary; }
	[[nodiscard]] auto is_section(JournalSection test) const -> bool { return m_section == test; }

  private:
	void on_open(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) override;
	void on_close(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) override;
	void refresh();
	void set_text();
	void switch_sections(automa::ServiceProvider& svc);

  private:
	JournalSection m_section{};
	sf::Vector2f m_questlog_position{};
	struct {
		sf::Text readout;
		std::vector<QuestEntry> listing{};
		std::optional<gui::TextWriter> objective{};
		float anim{};
		float desc_offset{};
	} m_text;

	std::optional<InventorySelector> m_selector{};
	sf::RenderTexture m_screen{};
	util::RectPath m_path;
	automa::ServiceProvider* m_services;
	sf::Sprite m_selector_sprite;
	Drawable m_indicator;
	int m_selected_quest{};

	float m_spacing{24.f};
};

} // namespace fornani::gui
