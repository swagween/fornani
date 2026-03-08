
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/Polymorphic.hpp>

namespace fornani::graphics {

enum class RewardSequenceFlags { show_player, console_after_exit };

class IRewardSequence : public Polymorphic {
  public:
	IRewardSequence(automa::ServiceProvider& svc, player::Player& player, world::Map& map);
	virtual void update(automa::ServiceProvider& svc, player::Player& player, world::Map& map);
	virtual void render(sf::RenderWindow& win);
	void set_label(std::string_view to) { p_label.emplace(to.data()); }

	[[nodiscard]] auto is_done() const -> bool { return p_end.is_almost_complete(); }
	[[nodiscard]] auto get_label() const -> std::optional<std::string_view> { return p_label; }

	util::BitFlags<RewardSequenceFlags> flags{};

  protected:
	util::Cooldown p_end;
	util::Cooldown p_start;
	util::Cooldown p_linger;

	std::optional<std::string> p_label{};
};

} // namespace fornani::graphics
