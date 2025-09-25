
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
	explicit Sound(capo::IEngine& engine, capo::Buffer const& buffer, std::string const& label, int echo_count = 0, int echo_rate = 16, float volume = 1.f, int fade = 16);
	void update(automa::ServiceProvider& svc);
	void play(bool repeat = false);
	void set_volume(float volume);
	void set_pitch(float pitch);
	void fade_out();
	void set_fading(bool fading) { m_fading = fading; }

	[[nodiscard]] auto get_label() const -> std::string { return m_label; }
	[[nodiscard]] auto is_echoing() const -> bool { return echo.count.running(); }
	[[nodiscard]] auto is_playing() const -> bool {
		return std::ranges::find_if(m_sounds, [](auto& s) { return s->is_playing(); }) != std::ranges::end(m_sounds);
	}
	[[nodiscard]] auto is_running() const -> bool { return (is_playing() || is_echoing()) && !delete_me; }

  private:
	util::Cooldown m_fade{};
	std::string m_label;
	float m_volume;
	bool delete_me{};
	bool m_fading{};
	bool m_fading_out{};
	std::vector<std::unique_ptr<capo::ISource>> m_sounds{};
	struct {
		util::Cooldown count{};
		util::Cooldown repeater{};
		int rate{};
	} echo{};

	io::Logger m_logger{"Audio"};
};

} // namespace fornani::audio
