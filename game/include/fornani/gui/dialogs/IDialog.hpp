
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/graphics/Drawable.hpp>
#include <fornani/graphics/HelpText.hpp>
#include <fornani/shader/Palette.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/Polymorphic.hpp>

namespace fornani {
class LightShader;
}

namespace fornani::graphics {
class Transition;
}

namespace fornani::gui {

enum class DialogStatus { opened, made_sale, closed, intro_done, waiting_to_enter, entered };
enum class DialogState { buy, sell, build };
enum class IDialogFlags { early_render_return, early_tick_return };

class IDialog : public UniquePolymorphic {
  public:
	explicit IDialog(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int vendor_id, std::string const& type);
	virtual void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, SceneContext& context);
	virtual void render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player, world::Map& map, LightShader& shader);
	virtual void close();
	virtual void refresh(automa::ServiceProvider& svc, player::Player& player, world::Map& map);

	[[nodiscard]] auto is_open() const -> bool { return p_flags.test(DialogStatus::opened); }
	[[nodiscard]] auto is_buying() const -> bool { return p_state == DialogState::buy; }
	[[nodiscard]] auto is_selling() const -> bool { return p_state == DialogState::sell; }
	[[nodiscard]] auto made_sale() const -> bool { return p_flags.test(DialogStatus::made_sale); }
	[[nodiscard]] auto made_profit() const -> bool { return p_balance > 0.f; }
	[[nodiscard]] auto is_opening() const -> bool { return m_intro.running() || p_flags.test(DialogStatus::intro_done); }
	[[nodiscard]] auto is_closing() const -> bool { return m_outro.running() || p_flags.test(DialogStatus::closed); }

  protected:
	[[nodiscard]] auto get_vendor_id() const -> int { return m_vendor_id; }
	[[nodiscard]] auto early_render_return() -> bool { return m_flags.consume(IDialogFlags::early_render_return); }
	[[nodiscard]] auto early_tick_return() -> bool { return m_flags.consume(IDialogFlags::early_tick_return); }

  protected:
	DialogState p_state{};
	util::BitFlags<DialogStatus> p_flags{};
	float p_balance{};
	float p_upcharge{};
	std::string p_npc_label{};
	Palette p_palette;

	std::optional<Drawable> p_artwork{};
	Drawable p_selector_sprite;
	Drawable p_vendor_portrait;

  private:
	bool fade_logic(automa::ServiceProvider& svc, graphics::Transition& transition);

  private:
	int m_vendor_id{};
	util::Cooldown m_intro;
	util::Cooldown m_fade_in;
	util::Cooldown m_outro;
	util::BitFlags<IDialogFlags> m_flags{};
	sf::RectangleShape m_background{};
	std::optional<graphics::HelpText> m_helptext{};
};

} // namespace fornani::gui
