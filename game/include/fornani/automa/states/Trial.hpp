
#include <fornani/automa/GameState.hpp>
#include <memory>
#include <optional>

namespace fornani::automa {

class Trial final : public GameState {
  public:
	Trial(ServiceProvider& svc, player::Player& player, std::string_view scene = "", int room_number = 0, std::string_view room_name = "");
	void tick_update(ServiceProvider& svc, capo::IEngine& engine) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, sf::RenderWindow& win) override;

	world::Map map;
	std::optional<std::unique_ptr<gui::PauseWindow>> pause_window{};

	ServiceProvider* m_services;

  private:
	util::Cooldown m_reset;
};

} // namespace fornani::automa
