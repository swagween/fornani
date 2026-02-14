
#pragma once

#include <SFML/System/Clock.hpp>
#include <deque>
#include <numeric>
#include <ranges>

namespace fornani {

class FrameTracker {
  public:
	void update() {
		m_frame_times.push_back(get_elapsed_time().asMilliseconds());
		if (m_frame_times.size() > m_sample_size) { m_frame_times.pop_front(); }
		m_delta_clock.restart();
	}
	void set_sample_size(std::size_t const to) { m_sample_size = to; }
	[[nodiscard]] auto get_elapsed_time() const -> sf::Time { return m_delta_clock.getElapsedTime(); }
	[[nodiscard]] auto get_average_frame_time() const -> double { return m_frame_times.empty() ? 0.0 : std::accumulate(m_frame_times.begin(), m_frame_times.end(), 0LL) / static_cast<double>(m_frame_times.size()); }

  private:
	std::size_t m_sample_size{256};
	sf::Clock m_delta_clock{};
	std::deque<std::int64_t> m_frame_times{};
};

} // namespace fornani
