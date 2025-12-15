
#pragma once

#include "fornani/automa/MenuState.hpp"
#include "fornani/gui/MiniMenu.hpp"

#include <optional>

namespace fornani::automa {

class FileMenu final : public MenuState {

	static constexpr int num_files{3};

  public:
	FileMenu(ServiceProvider& svc, player::Player& player);
	void tick_update(ServiceProvider& svc, capo::IEngine& engine) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, sf::RenderWindow& win) override;
	void refresh(ServiceProvider& svc);

  private:
	util::Cooldown loading{};
	std::optional<gui::MiniMenu> m_file_select_menu{};
};

} // namespace fornani::automa
