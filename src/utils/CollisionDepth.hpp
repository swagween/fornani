#pragma once
#include <deque>
#include <SFML/Graphics.hpp>

namespace shape {
class Shape;
}

namespace util {

struct Depth {
	float left{};
	float right{};
	float top{};
	float bottom{};
};

class CollisionDepth {
  public:
	CollisionDepth() = default;
	void calculate(shape::Shape const& native, shape::Shape const& other);
	Depth get() const { return out_depth; }
	void update();
	void print();
	void render(shape::Shape const& bounding_box, sf::RenderWindow& win, sf::Vector2<float> cam);
	[[nodiscard]] auto crushed() const -> bool { return (out_depth.bottom < -crush_threshold && out_depth.top > crush_threshold) || (out_depth.left > crush_threshold && out_depth.right < -crush_threshold); }
	[[nodiscard]] auto left_depth() const -> float { return out_depth.left; }
	[[nodiscard]] auto right_depth() const -> float { return out_depth.right; }
	[[nodiscard]] auto top_depth() const -> float { return out_depth.top; }
	[[nodiscard]] auto bottom_depth() const -> float { return out_depth.bottom; }
	
  private:
	void try_push();

	Depth candidate{};
	Depth out_depth{};
	std::deque<Depth> stream{};
	size_t stream_size{16};
	float crush_threshold{1.0f};
	float depth_throwaway{8.0f};
	float depth_maximum{12.0f};
	sf::RectangleShape collision_ray{};
};

} // namespace util