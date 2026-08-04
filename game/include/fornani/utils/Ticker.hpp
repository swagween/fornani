
#pragma once

#include <fornani/io/Logger.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <fornani/utils/Constants.hpp>
#include <fornani/utils/Math.hpp>
#include <fornani/utils/Timer.hpp>
#include <chrono>
#include <deque>
#include <thread>

namespace fornani::util {

constexpr static auto default_slowdown_rate_v = 1.f;
constexpr static auto max_integrations_v = 4;

using Clk = std::chrono::steady_clock;
using Sec = std::chrono::duration<float>;
using Mil = std::chrono::milliseconds;

enum class TickerFlags { forced_slowdown, paused };
enum class Period { second, twenty_minutes };
struct PeriodicBool {
	explicit PeriodicBool(Sec const period) : period(period) {}
	Sec period{};
	Sec elapsed{};
	constexpr bool tick(Sec const dt) {
		elapsed += dt;
		if (elapsed >= period) {
			elapsed = std::chrono::seconds{0};
			return true;
		}
		return false;
	}
};
class Ticker {
  public:
	template <typename F>
	void tick(F fn) {

		ft = Sec{tick_rate};
		manage_slowdowns();

		new_time = Clk::now();
		dt = std::chrono::duration_cast<Sec>(new_time - current_time);
		dt *= dt_scalar;
		current_time = new_time;

		if (dt_scalar < constants::tiny_value) { return; }

		if (dt.count() > tick_limit.count()) { return; } // return for unexpected dt values, particularly during the beginning of the state

		accumulator = dt + residue;
		if (accumulator < ft) {
			residue += accumulator;
			accumulator = Sec::zero();
			return;
		}
		integrations = 0;
		while (accumulator >= ft && integrations < max_integrations_v) {
			second_ticker.tick(ft) ? periods.set(Period::second) : periods.reset(Period::second);
			twenty_minute_ticker.tick(ft) ? periods.set(Period::twenty_minutes) : periods.reset(Period::twenty_minutes);
			fn();
			accumulator -= ft;
			++integrations;
			++total_integrations;
			++ticks;
		}
		if (integrations == max_integrations_v) { residue = Sec::zero(); }

		residue = accumulator;
		accumulator = Sec::zero();
		++calls_per_frame;
	};
	void reset();

	void start_frame();
	void end_frame();
	void calculate_fps();
	void slow_down(float time, float target = 0.5f, float rate = default_slowdown_rate_v);
	void freeze_frame(float time, float rate = default_slowdown_rate_v);
	void set_time(Sec time);
	void scale_dt();
	void reset_dt();
	void pause() { flags.set(TickerFlags::paused); }
	void unpause() { flags.reset(TickerFlags::paused); }
	[[nodiscard]] auto paused() const -> bool { return flags.test(TickerFlags::paused); }

	[[nodiscard]] auto global_tick_rate() const -> float;
	[[nodiscard]] auto every_x_frames(int const freq) const -> bool { return num_frames % freq == 0; }
	[[nodiscard]] auto every_x_ticks(int const freq) const -> bool { return ticks % freq == 0; }
	[[nodiscard]] auto every_second() const -> bool { return periods.test(Period::second); }
	[[nodiscard]] auto every_twenty_minutes() const -> bool { return periods.test(Period::twenty_minutes); }

	[[nodiscard]] auto get_freezeframe() const -> float { return freezeframe.normalized(); }
	[[nodiscard]] auto get_slowdown() const -> float { return slowdown.normalized(); }

	Clk::time_point current_time{Clk::now()};
	Clk::time_point new_time{Clk::now()};

	float tick_rate{0.005f};
	float tick_multiplier{24.f};
	float dt_scalar{1.f};
	float global_scalar{1.f};

	static constexpr Sec tick_limit{0.8f};

	Sec ft{};
	Sec dt{};
	Sec accumulator{};
	Sec residue{};

	// for TPS and FPS calculations
	int integrations{};
	int ticks{};
	int calls_per_frame{};
	int sample_size{256};
	int num_frames{};
	float total_integrations{};
	float ticks_per_frame{};

	Sec seconds_passed{};
	Sec total_seconds_passed{};
	Sec in_game_seconds_passed{};
	Sec total_milliseconds_passed{};
	PeriodicBool second_ticker{std::chrono::seconds{1}};
	PeriodicBool twenty_minute_ticker{std::chrono::seconds{1200}};
	float fps{60.f};

  private:
	void manage_slowdowns();

  private:
	std::deque<Sec> frame_list{};
	BitFlags<TickerFlags> flags{};
	BitFlags<Period> periods{};
	float slowdown_target{};
	float slowdown_rate{};
	Timer slowdown{};
	Timer freezeframe{};

	io::Logger m_logger{"Ticker"};
};

} // namespace fornani::util
