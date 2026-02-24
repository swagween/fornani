
#pragma once

#include <fornani/automa/GameplayState.hpp>
#include <fornani/events/Subscription.hpp>
#include <fornani/shader/LightShader.hpp>
#include <fornani/world/Camera.hpp>
#include <memory>
#include <optional>

namespace fornani::automa {

enum class GameplayFlags { game_over, transitioning, open_vendor, give_item, item_music_played, death_console_launched, console_running, remove_item };

class Dojo final : public GameplayState {
  public:
	Dojo(ServiceProvider& svc, player::Player& player, std::string_view scene = "", int room_number = 0, std::string_view room_name = "");
	void tick_update(ServiceProvider& svc, capo::IEngine& engine) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, sf::RenderWindow& win) override;
	void reload(ServiceProvider& svc, int target_state) override;

	void acquire_item(ServiceProvider& svc, std::string_view tag);
	void remove_item(ServiceProvider& svc, std::string_view tag);
	void acquire_gun(ServiceProvider& svc, std::string_view tag);
	void remove_gun(ServiceProvider& svc, std::string_view tag);
	void acquire_item_from_console(ServiceProvider& svc, int id);
	void acquire_gun_from_console(ServiceProvider& svc, int id);
	void remove_gun_by_id(ServiceProvider& svc, int id);
	void remove_item_by_id(ServiceProvider& svc, int id);
	void equip_item(ServiceProvider& svc, int id);
	void open_vendor(ServiceProvider& svc, int id);
	void launch_cutscene(ServiceProvider& svc, int id);
	void add_map_marker(ServiceProvider& svc, int room_id, int type, int questline);
	bool check_for_vendor(ServiceProvider& svc);
	void read_item(int id);
	void handle_player_death(ServiceProvider& svc, player::Player& player);

	bool show_colliders{false};

  private:
	util::BitFlags<GameplayFlags> m_flags{};

	int m_vendor_id{};
	std::string m_item_tag{};

	std::unordered_map<int, std::string> m_map_markers{};

	util::Cooldown m_enter_room;
	util::Cooldown m_loading;
	std::optional<util::Cooldown> m_inspect_hint{};
	std::optional<util::Cooldown> m_shoot_hint{};
};

} // namespace fornani::automa
