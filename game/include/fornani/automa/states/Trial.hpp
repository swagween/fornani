
#include <fornani/automa/GameState.hpp>
#include <fornani/shader/LightShader.hpp>
#include <memory>
#include <optional>

namespace fornani::automa {

class Trial final : public GameState {
  public:
	Trial(ServiceProvider& svc, player::Player& player, std::string_view scene = "", int room_number = 0, std::string_view room_name = "");
	void tick_update(ServiceProvider& svc, capo::IEngine& engine) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, sf::RenderWindow& win) override;
	std::optional<std::reference_wrapper<world::Map>> get_map() override { return map; }

  private:
	util::Cooldown m_reset;
	std::optional<LightShader> m_shader{};
	ServiceProvider* m_services;
	world::Map map;
	std::optional<std::unique_ptr<gui::PauseWindow>> pause_window{};
};

} // namespace fornani::automa
