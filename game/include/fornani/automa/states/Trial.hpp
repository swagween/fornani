
#include <fornani/automa/GameplayState.hpp>
#include <fornani/shader/LightShader.hpp>
#include <memory>
#include <optional>

namespace fornani::automa {

class Trial final : public GameplayState {
  public:
	Trial(ServiceProvider& svc, player::Player& player, int room_number);
	void tick_update(ServiceProvider& svc, capo::IEngine& engine) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, sf::RenderWindow& win) override;
	void reload(ServiceProvider& svc, int target_state) override;

  private:
	virtual void pause(ServiceProvider& svc) override;
	util::Cooldown m_reset;
};

} // namespace fornani::automa
