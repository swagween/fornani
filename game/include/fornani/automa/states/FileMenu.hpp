
#pragma once

#include "fornani/automa/GameState.hpp"
#include "fornani/gui/MiniMenu.hpp"

namespace fornani::automa {

class FileMenu final : public GameState {

	static constexpr int num_files{3};

  public:
	FileMenu(ServiceProvider& svc, player::Player& player, std::string_view scene = "", int room_number = 0);
	void tick_update(ServiceProvider& svc) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, sf::RenderWindow& win) override;
	void refresh(ServiceProvider& svc);

	sf::RectangleShape title{};
private:
	util::Cooldown loading{};
	world::Map map;
	gui::MiniMenu file_select_menu;
};

}
