
#pragma once

#include <SFML/Graphics.hpp>
#include <cstdio>
#include <memory>
#include <random>

namespace util {

class Random {

  public:
	int random_range(int lo, int hi) { return std::uniform_int_distribution<int>{lo, hi}(engine); }
	int random_range(int lo, int hi, int seed) {
		seeded_engine.seed(seed);
		return std::uniform_int_distribution<int>{lo, hi}(seeded_engine);
	}

	float random_range_float(float lo, float hi) { return std::uniform_real_distribution<float>{lo, hi}(engine); }

	sf::Vector2<float> random_vector_float(float lo, float hi) {
		auto randx = random_range_float(lo, hi);
		auto randy = random_range_float(lo, hi);
		return {randx, randy};
	}

	sf::Vector2<float> random_vector_float(sf::Vector2<float> lo, sf::Vector2<float> hi) {
		auto randx = random_range_float(lo.x, hi.x);
		auto randy = random_range_float(lo.y, hi.y);
		return {randx, randy};
	}

	int unsigned_coin_flip() {
		auto result = std::uniform_real_distribution<float>{-1.0f, 1.0f}(engine);
		return (result > 0.0f) ? 0 : 1;
	}

	int signed_coin_flip() {
		auto result = std::uniform_real_distribution<float>{-1.0f, 1.0f}(engine);
		return (result > 0.0f) ? -1 : 1;
	}

	float random_range_normal(float const mean, float const std_dev) { return std::normal_distribution<float>{mean, std_dev}(engine); }

	bool percent_chance(float const percent) {
		auto result = std::uniform_real_distribution<float>{0.0f, 100.0f}(engine);
		return result < percent;
	}

	int get_vendor_seed() const { return seeds.vendor; }
	int get_test_seed() const { return seeds.test; }

	void set_test_seed() { seeds.test = random_range(0, 100000); }
	void set_vendor_seed() { seeds.vendor = random_range(0, 100000); }

  private:
	std::default_random_engine engine{std::random_device{}()};
	std::default_random_engine seeded_engine{std::random_device{}()};
	struct {
		int vendor{1997};
		int test{2007};
	} seeds{};
};

} // namespace util