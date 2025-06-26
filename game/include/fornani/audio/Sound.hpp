
#pragma once

#include <capo/engine.hpp>
#include <fornani/io/Logger.hpp>
#include <ranges>
#include <string>
#include "fornani/utils/Cooldown.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::audio {

class Sound {
  public:
	explicit Sound(capo::IEngine& engine, capo::Buffer const& buffer, std::string const& label, int echo_count = 0, int echo_rate = 16);
	void update(automa::ServiceProvider& svc);
	void play(bool repeat = false);
	void set_volume(float volume);
	void set_pitch(float pitch);
	[[nodiscard]] auto get_label() const -> std::string { return m_label; }
	[[nodiscard]] auto is_echoing() const -> bool { return echo.count.running(); }
	[[nodiscard]] auto is_playing() const -> bool {
		return std::ranges::find_if(m_sounds, [](auto& s) { return s->is_playing(); }) == std::ranges::end(m_sounds);
	}
	[[nodiscard]] auto is_running() const -> bool { return is_playing() || is_echoing(); }

  private:
	std::string m_label;
	float native_volume{100.f};
	std::vector<std::unique_ptr<capo::ISource>> m_sounds{};
	struct {
		util::Cooldown count{};
		util::Cooldown repeater{};
		int rate{};
	} echo{};

	io::Logger m_logger{"Audio"};
};

} // namespace fornani::audio
