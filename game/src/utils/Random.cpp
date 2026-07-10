
#include <fornani/utils/Random.hpp>
#include <numbers>

namespace fornani::random {

struct {
	seed_t vendor = 0x9E3779B9u; // golden ratio constant
	seed_t test = 0x85EBCA6Bu;	 // MurmurHash3 mix constant
} seeds{};

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

sf::Vector2f random_weighted_offset(float radius, float bias) {
	static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
	float const angle = dist(engine()) * 2.0f * std::numbers::pi_v<float>;
	float r = dist(engine());
	r = std::pow(r, bias); // bias > 1 → more weight toward center
	float const final_radius = r * radius;
	return {std::cos(angle) * final_radius, std::sin(angle) * final_radius};
}

int unsigned_coin_flip() { return std::bernoulli_distribution(0.5)(engine()) ? 1 : 0; }

int signed_coin_flip() { return std::bernoulli_distribution(0.5)(engine()) ? 1 : -1; }

bool coin_flip() { return static_cast<bool>(unsigned_coin_flip()); }

// Generates a random float following a normal distribution with the given mean and standard deviation
float random_range_normal(float mean, float std_dev) { return std::normal_distribution<float>{mean, std_dev}(engine()); }

// Returns true with a probability corresponding to the provided percent chance
bool percent_chance(float percent) { return std::uniform_real_distribution<float>{0.0f, 100.0f}(engine()) < percent; }

seed_t get_vendor_seed() { return seeds.vendor; }

seed_t get_test_seed() { return seeds.test; }

void reset_vendor_seed() {
	seeds.vendor = random_range(0u, std::numeric_limits<int>::max());
	engine().seed(seeds.vendor);
}

void set_vendor_seed(seed_t const to) {
	seeds.vendor = to;
	engine().seed(seeds.vendor);
}

void set_test_seed() {
	seeds.test = random_range(0u, std::numeric_limits<int>::max());
	engine().seed(seeds.test);
}

} // namespace fornani::random
