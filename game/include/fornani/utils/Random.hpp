#pragma once

#include <SFML/Graphics.hpp>
#include <random>

namespace fornani::random {

int random_range(int lo, int hi);

// Generates a random integer in the range [lo, hi] using a provided seed
int random_range(int lo, int hi, int seed);

// Generates a random float in the range [lo, hi]
float random_range_float(float lo, float hi);

// Generates a random 2D vector of floats with both components in the range [lo, hi]
sf::Vector2f random_vector_float(float lo, float hi);

// Generates a random 2D vector of floats with x in [lo.x, hi.x] and y in [lo.y, hi.y]
sf::Vector2f random_vector_float(sf::Vector2f lo, sf::Vector2f hi);

// Returns 0 or 1 based on a coin flip using an unsigned logic
int unsigned_coin_flip();

// Returns -1 or 1 based on a coin flip using a signed logic
int signed_coin_flip();

// Generates a random float following a normal distribution with the given mean and standard deviation
float random_range_normal(float mean, float std_dev);

// Returns true with a probability corresponding to the provided percent chance
bool percent_chance(float percent);

// Accessor for the vendor seed
int get_vendor_seed();

// Accessor for the test seed
int get_test_seed();

// Sets a new test seed
void set_test_seed();

// Sets a new vendor seed
void set_vendor_seed();

} // namespace fornani::random
