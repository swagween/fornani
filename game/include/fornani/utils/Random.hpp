
#pragma once

#include <SFML/Graphics.hpp>
#include <random>

namespace fornani::util {

struct Random {
	static int random_range(int lo, int hi) { return std::uniform_int_distribution<int>{lo, hi}(engine); }

	// Generates a random integer in the range [lo, hi] using a provided seed
	static int random_range(int lo, int hi, int seed) {
		seeded_engine.seed(seed);
		return std::uniform_int_distribution<int>{lo, hi}(seeded_engine);
	}

	// Generates a random float in the range [lo, hi]
	static float random_range_float(float lo, float hi) { return std::uniform_real_distribution<float>{lo, hi}(engine); }

	// Generates a random 2D vector of floats with both components in the range [lo, hi]
	static sf::Vector2<float> random_vector_float(float lo, float hi) {
		auto randx = random_range_float(lo, hi);
		auto randy = random_range_float(lo, hi);
		return {randx, randy};
	}

	// Generates a random 2D vector of floats with x in [lo.x, hi.x] and y in [lo.y, hi.y]
	static sf::Vector2<float> random_vector_float(sf::Vector2<float> lo, sf::Vector2<float> hi) {
		auto randx = random_range_float(lo.x, hi.x);
		auto randy = random_range_float(lo.y, hi.y);
		return {randx, randy};
	}

	// Returns 0 or 1 based on a coin flip using an unsigned logic
	static int unsigned_coin_flip() {
		auto const result = std::uniform_real_distribution<float>{-1.0f, 1.0f}(engine);
		return (result > 0.0f) ? 0 : 1;
	}

	// Returns -1 or 1 based on a coin flip using a signed logic
	static int signed_coin_flip() {
		auto const result = std::uniform_real_distribution<float>{-1.0f, 1.0f}(engine);
		return (result > 0.0f) ? -1 : 1;
	}

	// Generates a random float following a normal distribution with the given mean and standard deviation
	static float random_range_normal(float mean, float std_dev) { return std::normal_distribution<float>{mean, std_dev}(engine); }

	// Returns true with a probability corresponding to the provided percent chance
	static bool percent_chance(float percent) {
		auto const result = std::uniform_real_distribution<float>{0.0f, 100.0f}(engine);
		return result < percent;
	}

	// Accessor for the vendor seed
	static int get_vendor_seed() { return seeds.vendor; }

	// Accessor for the test seed
	static int get_test_seed() { return seeds.test; }

	// Sets a new test seed
	static void set_test_seed() { seeds.test = random_range(0, 100000); }

	// Sets a new vendor seed
	static void set_vendor_seed() { seeds.vendor = random_range(0, 100000); }

  private:
	inline static std::default_random_engine engine{std::random_device{}()};
	inline static std::default_random_engine seeded_engine{std::random_device{}()};
	inline static struct {
		int vendor;
		int test;
	} seeds = {1997, 2007};
};

} // namespace fornani::util
