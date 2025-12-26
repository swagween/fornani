
#include <fornani/utils/Random.hpp>

namespace fornani::random {

namespace {

struct {
	int vendor;
	int test;
} seeds = {1997, 2007};

} // namespace

// Generates a random integer in the range [lo, hi] using a provided seed
int random_range(int lo, int hi) { return std::uniform_int_distribution<int>{lo, hi}(engine()); }

int random_range(int lo, int hi, std::uint32_t seed) {
	std::mt19937 gen(seed);
	return std::uniform_int_distribution<int>{lo, hi}(gen);
}

// Generates a random float in the range [lo, hi]
float random_range_float(float lo, float hi) { return std::uniform_real_distribution<float>{lo, hi}(engine()); }

// Generates a random 2D vector of floats with both components in the range [lo, hi]
sf::Vector2f random_vector_float(float lo, float hi) {
	auto randx = random_range_float(lo, hi);
	auto randy = random_range_float(lo, hi);
	return {randx, randy};
}

// Generates a random 2D vector of floats with x in [lo.x, hi.x] and y in [lo.y, hi.y]
sf::Vector2f random_vector_float(sf::Vector2f lo, sf::Vector2f hi) {
	auto randx = random_range_float(lo.x, hi.x);
	auto randy = random_range_float(lo.y, hi.y);
	return {randx, randy};
}

int unsigned_coin_flip() { return std::bernoulli_distribution(0.5)(engine()) ? 1 : 0; }

int signed_coin_flip() { return std::bernoulli_distribution(0.5)(engine()) ? 1 : -1; }

// Generates a random float following a normal distribution with the given mean and standard deviation
float random_range_normal(float mean, float std_dev) { return std::normal_distribution<float>{mean, std_dev}(engine()); }

// Returns true with a probability corresponding to the provided percent chance
bool percent_chance(float percent) { return std::uniform_real_distribution<float>{0.0f, 100.0f}(engine()) < percent; }

int get_vendor_seed() { return static_cast<int>(seeds.vendor); }

int get_test_seed() { return static_cast<int>(seeds.test); }

void set_vendor_seed() {
	seeds.vendor = static_cast<uint32_t>(random_range(0, 100000));
	engine().seed(seeds.vendor);
}

void set_test_seed() {
	seeds.test = static_cast<uint32_t>(random_range(0, 100000));
	engine().seed(seeds.test);
}

} // namespace fornani::random
