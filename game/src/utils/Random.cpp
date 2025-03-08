#include "fornani/utils/Random.hpp"

namespace fornani::util {


namespace random {
namespace {
  std::default_random_engine engine{std::random_device{}()};
  std::default_random_engine seeded_engine{std::random_device{}()};
}

	inline struct {
		int vendor;
		int test;
	} seeds = {1997, 2007};

	int random_range(int lo, int hi) { return std::uniform_int_distribution<int>{lo, hi}(engine); }

	// Generates a random integer in the range [lo, hi] using a provided seed
	int random_range(int lo, int hi, int seed) {
		seeded_engine.seed(seed);
		return std::uniform_int_distribution<int>{lo, hi}(seeded_engine);
	}

	// Generates a random float in the range [lo, hi]
	float random_range_float(float lo, float hi) { return std::uniform_real_distribution<float>{lo, hi}(engine); }

	// Generates a random 2D vector of floats with both components in the range [lo, hi]
	sf::Vector2<float> random_vector_float(float lo, float hi) {
		auto randx = random_range_float(lo, hi);
		auto randy = random_range_float(lo, hi);
		return {randx, randy};
	}

	// Generates a random 2D vector of floats with x in [lo.x, hi.x] and y in [lo.y, hi.y]
	sf::Vector2<float> random_vector_float(sf::Vector2<float> lo, sf::Vector2<float> hi) {
		auto randx = random_range_float(lo.x, hi.x);
		auto randy = random_range_float(lo.y, hi.y);
		return {randx, randy};
	}

	// Returns 0 or 1 based on a coin flip using an unsigned logic
	int unsigned_coin_flip() {
		auto const result = std::uniform_real_distribution<float>{-1.0f, 1.0f}(engine);
		return (result > 0.0f) ? 0 : 1;
	}

	// Returns -1 or 1 based on a coin flip using a signed logic
	int signed_coin_flip() {
		auto const result = std::uniform_real_distribution<float>{-1.0f, 1.0f}(engine);
		return (result > 0.0f) ? -1 : 1;
	}

	// Generates a random float following a normal distribution with the given mean and standard deviation
	float random_range_normal(float mean, float std_dev) { return std::normal_distribution<float>{mean, std_dev}(engine); }

	// Returns true with a probability corresponding to the provided percent chance
	bool percent_chance(float percent) {
		auto const result = std::uniform_real_distribution<float>{0.0f, 100.0f}(engine);
		return result < percent;
	}

	// Accessor for the vendor seed
	int get_vendor_seed() { return seeds.vendor; }

	// Accessor for the test seed
	int get_test_seed() { return seeds.test; }

	// Sets a new test seed
	void set_test_seed() { seeds.test = random_range(0, 100000); }

	// Sets a new vendor seed
	void set_vendor_seed() { seeds.vendor = random_range(0, 100000); }
}
} // namespace fornani::util
