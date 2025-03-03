#pragma once
#include <SFML/Graphics.hpp>
#include <deque>
#include "Counter.hpp"

namespace fornani::shape {
class Shape;
class Collider;
} // namespace fornani::shape

namespace fornani::util {

enum class CollisionDirection : std::uint8_t { none, vertical, horizontal };

struct Depth {
	float left{};
	float right{};
	float top{};
	float bottom{};
	[[nodiscard]] auto total() const -> float { return left + right + top + bottom; }
};

class CollisionDepth {
  public:
	CollisionDepth() = default;
	void calculate(shape::Collider const& native, shape::Shape const& other);
	Depth get() const { return out_depth; }
	void update();
	void maximize(CollisionDepth& other);
	void print();
	void reset() { iterations.start(); }
	void render(shape::Shape const& bounding_box, sf::RenderWindow& win, sf::Vector2<float> cam);
	[[nodiscard]] auto crushed() const -> bool { return (out_depth.bottom < -crush_threshold && out_depth.top > crush_threshold) || (out_depth.left > crush_threshold && out_depth.right < -crush_threshold); }
	[[nodiscard]] bool horizontal_squish() const;
	[[nodiscard]] bool vertical_squish() const;

	[[nodiscard]] auto left_depth() const -> float { return out_depth.left; }
	[[nodiscard]] auto right_depth() const -> float { return out_depth.right; }
	[[nodiscard]] auto top_depth() const -> float { return out_depth.top; }
	[[nodiscard]] auto bottom_depth() const -> float { return out_depth.bottom; }

  private:
	void try_push();

	CollisionDirection collision_direction{};
	Counter iterations{};
	Depth candidate{};
	Depth out_depth{};
	std::deque<Depth> stream{};
	size_t stream_size{16};
	float crush_threshold{8.0f};
	float depth_throwaway{12.0f};
	float depth_maximum{12.0f};
	sf::RectangleShape collision_ray{};
};

} // namespace fornani::util