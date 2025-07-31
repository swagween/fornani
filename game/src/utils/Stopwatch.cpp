
#include <fornani/utils/Stopwatch.hpp>

namespace fornani::util {

void Stopwatch::start() {
	m_start_time = Clock::now();
	m_paused_time = {};
	m_stored_time = {};
}

void Stopwatch::resume() {
	m_start_time = Clock::now();
	m_paused_time += Clock::now() - m_end_time;
}

void Stopwatch::stop() {
	m_end_time = Clock::now();
	m_stored_time = get_elapsed_time();
}

void Stopwatch::print_time() const { NANI_LOG_DEBUG(m_logger, "Elapsed Time: {:.5f} seconds", as_seconds().count()); }

[[nodiscard]] auto Stopwatch::get_elapsed_time() const -> DurationSeconds { return std::chrono::duration_cast<DurationSeconds>(Clock::now() - m_start_time + m_stored_time); }

} // namespace fornani::util
