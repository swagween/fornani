
#include <fornani/automa/GameplayState.hpp>
#include <fornani/shader/LightShader.hpp>
#include <memory>
#include <optional>

namespace fornani::automa {

class Trial final : public GameplayState {
  public:
	Trial(ServiceProvider& svc, player::Player& player, std::string_view scene = "", int room_number = 0, std::string_view room_name = "");
	void tick_update(ServiceProvider& svc, capo::IEngine& engine) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, sf::RenderWindow& win) override;

  private:
	virtual void pause(ServiceProvider& svc) override;
	util::Cooldown m_reset;
};

} // namespace fornani::automa
