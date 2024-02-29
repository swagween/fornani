
#pragma once

#include <chrono>
#include <memory>
#include <random>

namespace util {

class Stopwatch {

  public:
	using Clk = std::chrono::steady_clock;
	using Tpt = std::chrono::time_point<std::chrono::steady_clock>;
	using Time = std::chrono::duration<float>;

	void start() { current_time = Clk::now(); };
	void stop() { elapsed_time = current_time - Clk::now(); };
	void print_time() { std::cout << "Elapsed Time: " << elapsed_time.count() << "\n"; }

	Time elapsed_time{};
	Tpt current_time = Clk::now();
};

} // namespace util