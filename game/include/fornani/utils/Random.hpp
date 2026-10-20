
#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cassert>
#include <random>
#include <ranges>
#include <vector>

namespace fornani::random {

using seed_t = std::uint32_t;

static std::mt19937& engine() {
	static std::mt19937 gen{std::random_device{}()};
	return gen;
}

int range(int lo, int hi);

// Generates a random integer in the range [lo, hi] using a provided seed
int range(int lo, int hi, std::uint32_t seed);

// Generates a random float in the range [lo, hi]
float range_float(float lo, float hi);

// Generates a random 2D vector of floats with both components in the range [lo, hi]
sf::Vector2f vector_float(float lo, float hi);

// Generates a random 2D vector of floats with x in [lo.x, hi.x] and y in [lo.y, hi.y]
sf::Vector2f vector_float(sf::Vector2f lo, sf::Vector2f hi);

// Generates a random vector from a point within `radius` distance, skewed toward the center
sf::Vector2f weighted_offset(float radius, float bias = 2.0f);

// Grabs a random element from a vector, excluding specified elements.
template <typename T>
T& element(std::vector<T>& v, std::vector<T> const& exclusions = {}) {
	assert(!v.empty() && "element: empty vector");

	std::vector<std::size_t> candidates;
	for (std::size_t i = 0; i < v.size(); ++i) {
		if (std::ranges::find(exclusions, v[i]) == exclusions.end()) { candidates.push_back(i); }
	}

	assert(!candidates.empty() && "element: no eligible elements");

	std::uniform_int_distribution<std::size_t> dist(0, candidates.size() - 1);
	return v[candidates[dist(engine())]];
}

template <typename T>
T const& element(std::vector<T> const& v, std::vector<T> const& exclusions = {}) {
	assert(!v.empty() && "element: empty vector");

	std::vector<std::size_t> candidates;
	for (std::size_t i = 0; i < v.size(); ++i) {
		if (std::ranges::find(exclusions, v[i]) == exclusions.end()) { candidates.push_back(i); }
	}

	assert(!candidates.empty() && "element: no eligible elements");

	std::uniform_int_distribution<std::size_t> dist(0, candidates.size() - 1);
	return v[candidates[dist(engine())]];
}

template <typename T, std::size_t Extent>
T const& element(std::span<T const, Extent> v) {
	assert(!v.empty() && "element: empty span");

	std::uniform_int_distribution<std::size_t> dist(0, v.size() - 1);
	return v[dist(engine())];
}

// Returns 0 or 1 based on a coin flip using an unsigned logic
int unsigned_coin_flip();

// Returns -1 or 1 based on a coin flip using a signed logic
int signed_coin_flip();

// Returns -1 or 1 based on a coin flip using a signed logic
bool coin_flip();

// Generates a random float following a normal distribution with the given mean and standard deviation
float range_normal(float mean, float std_dev);

// Returns true with a probability corresponding to the provided percent chance
bool percent_chance(float percent);

// Accessor for the vendor seed
seed_t get_vendor_seed();

// Accessor for the test seed
seed_t get_test_seed();

// Sets a new test seed
void set_test_seed();

// Sets a new vendor seed
void reset_vendor_seed();
void set_vendor_seed(seed_t const to);

template <typename T, typename WeightFn>
static T const& weighted_choice(std::vector<T> const& items, WeightFn weightFn) {
	std::vector<double> weights;
	weights.reserve(items.size());

	for (auto const& item : items) weights.push_back(weightFn(item));

	std::discrete_distribution<> dist(weights.begin(), weights.end());
	return items[dist(engine())];
}

} // namespace fornani::random
