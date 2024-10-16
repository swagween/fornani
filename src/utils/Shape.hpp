
#pragma once

#include <math.h>
#include <stdio.h>
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <vector>

namespace shape {

float const error = 0.0001f;

class Shape {

  public:
	using Vec = sf::Vector2<float>;

	Shape(Vec dim = {32.f, 32.f}, int num_vertices = 4);

	void set_position(const Vec new_pos);
	void update();

	Vec get_center();

	Vec perp(Vec edg);

	float getLength(Vec const v);
	Vec getNormalized(const Vec v);
	float dotProduct(const Vec a, const Vec b);
	Vec getNormal(const Vec v);
	Vec projectOnAxis(const std::vector<Vec> vertices, const Vec axis);
	bool areOverlapping(Vec const& a, Vec const& b);
	float getOverlapLength(Vec const& a, Vec const& b);
	Vec getCenter(Shape const& shape);
	Vec getThisCenter();
	std::vector<Vec> getVertices(Shape const& shape);
	Vec testCollisionGetMTV(Shape const& obb1, Shape const& obb2);
	bool SAT(Shape const& other);
	bool overlaps(Shape const& other) const;
	bool contains_point(Vec point);
	void render(sf::RenderWindow& win, sf::Vector2<float> cam);

	[[nodiscard]] auto left() const -> float { return position.x; }
	[[nodiscard]] auto right() const -> float { return position.x + dimensions.x; }
	[[nodiscard]] auto top() const -> float { return position.y; }
	[[nodiscard]] auto bottom() const -> float { return position.y + dimensions.y; }
	[[nodiscard]] auto get_center() const -> sf::Vector2<float> { return position + dimensions * 0.5f; }
	[[nodiscard]] float get_height_at(float x) const;

	bool AABB_handle_left_collision_static(Shape const& immovable);
	bool AABB_handle_right_collision_static(Shape const& immovable);
	bool AABB_is_left_collision(Shape const& immovable);
	bool AABB_is_right_collision(Shape const& immovable);

	std::vector<Vec> vertices{};
	std::vector<Vec> edges{};
	std::vector<Vec> normals{};
	Vec axis{};

	std::vector<sf::Vertex> draw_vertices{};

	// for hurtboxes
	Vec dimensions{};
	Vec position{};
	int tile_id{};
	int num_sides{};
};

} // namespace shape
