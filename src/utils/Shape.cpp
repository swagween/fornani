
#include "Shape.hpp"
#include <ccmath/math/power/sqrt.hpp>
#include <iostream>

namespace shape {

Shape::Vec Shape::perp(Vec edg) {
	Vec temp = Vec(-edg.y, edg.x);
	float mag;
	float a_squared = temp.x * temp.x;
	float b_squared = temp.y * temp.y;
	mag = sqrt(a_squared + b_squared);
	if (abs(mag) > 0) {
		temp.x = temp.x / mag;
		temp.y = temp.y / mag;
	}
	return temp;
}

Shape::Shape(Vec dim, int num_vertices) : dimensions(dim), num_sides(num_vertices) {
	for (int i = 0; i < num_vertices; i++) {
		edges.push_back({});
		normals.push_back({});
	}
	vertices.push_back({});
	vertices.push_back({dim.x, 0.f});
	vertices.push_back({dim.x, dim.y});
	if (num_vertices > 3) { vertices.push_back({0.f, dim.y}); }
	for (int i = 0; i < vertices.size(); i++) {
		edges[i].x = vertices[static_cast<size_t>(i + 1) % vertices.size()].x - vertices[i].x;
		edges[i].y = vertices[static_cast<size_t>(i + 1) % vertices.size()].y - vertices[i].y;
		normals[i] = perp(edges[i]);
		draw_vertices.push_back(sf::Vertex(vertices[0], sf::Color::Red));
	}
}

void Shape::set_position(const Vec new_pos) {
	position = new_pos;
	update();
}

void Shape::update() {
	if (vertices.size() >= 4) {
		vertices[0].x = position.x;
		vertices[0].y = position.y;
		vertices[1].x = position.x + dimensions.x;
		vertices[1].y = position.y;
		vertices[2].x = position.x + dimensions.x;
		vertices[2].y = position.y + dimensions.y;
		vertices[3].x = position.x;
		vertices[3].y = position.y + dimensions.y;
	}
	for (int i = 0; i < vertices.size(); i++) {
		edges[i].x = vertices[(i + 1) % vertices.size()].x - vertices[i].x;
		edges[i].y = vertices[(i + 1) % vertices.size()].y - vertices[i].y;
		normals[i] = perp(edges[i]);
	}
}

Shape::Vec Shape::get_center() {
	Vec result{};
	if (!vertices.empty()) {
		float x1 = vertices.front().x;
		float y1 = vertices.front().y;
		float x2 = vertices.back().x;
		float y2 = vertices.back().y;
		result.x = (x1 + x2) / 2.f;
		result.y = (y1 + y2) / 2.f;
	}
	return result;
}

static float const NORMAL_TOLERANCE = 0.0001f;

float Shape::getLength(const Vec v) { return ccm::sqrt(v.x * v.x + v.y * v.y); }

// Returns normalized vector
Shape::Vec Shape::getNormalized(const Vec v) {
	float length = getLength(v);
	if (length < NORMAL_TOLERANCE) { return Vec(); }
	return Vec(v.x / length, v.y / length);
}

float Shape::dotProduct(const Vec a, const Vec b) { return a.x * b.x + a.y * b.y; }

// Returns right hand perpendicular vector
Shape::Vec Shape::getNormal(const Vec v) { return Vec(-v.y, v.x); }

// Find minimum and maximum projections of each vertex on the axis
Shape::Vec Shape::projectOnAxis(const std::vector<Vec> vertices, const Vec axis) {
	float min = std::numeric_limits<float>::infinity();
	float max = -std::numeric_limits<float>::infinity();
	for (auto& vertex : vertices) {
		float projection = dotProduct(vertex, axis);
		if (projection < min) { min = projection; }
		if (projection > max) { max = projection; }
	}
	return Vec(min, max);
}

// a and b are ranges and it's assumed that a.x <= a.y and b.x <= b.y
bool Shape::areOverlapping(Vec const& a, Vec const& b) { return a.x <= b.y && a.y >= b.x; }

// a and b are ranges and it's assumed that a.x <= a.y and b.x <= b.y
float Shape::getOverlapLength(Vec const& a, Vec const& b) {
	if (!areOverlapping(a, b)) { return 0.f; }
	return std::min(a.y, b.y) - std::max(a.x, b.x);
}

Shape::Vec Shape::getCenter(Shape const& shape) { return (Vec(shape.position.x + shape.dimensions.x / 2.f, shape.position.y + shape.dimensions.y / 2.f)); }

Shape::Vec Shape::getThisCenter() { return (Vec(position.x + dimensions.x / 2.f, position.y + dimensions.y / 2.f)); }

std::vector<Shape::Vec> Shape::getVertices(Shape const& shape) { return shape.vertices; }

Shape::Vec Shape::testCollisionGetMTV(Shape const& obb1, Shape const& obb2) {
	auto t_mtv = Vec{};
	auto& vertices1 = vertices;
	auto vertices2 = getVertices(obb2);

	auto& axes1 = normals;
	auto& axes2 = obb2.normals;

	// we need to find the minimal overlap and axis on which it happens
	auto minOverlap = std::numeric_limits<float>::max();

	for (auto& axis : axes1) {
		Vec proj1 = projectOnAxis(vertices1, axis);
		Vec proj2 = projectOnAxis(vertices2, axis);

		float overlap = getOverlapLength(proj1, proj2);
		if (overlap < NORMAL_TOLERANCE) { // shapes are not overlapping
			return {};
		} else {
			if (overlap < minOverlap) {
				minOverlap = overlap;
				t_mtv.x = axis.x * minOverlap;
				t_mtv.y = axis.y * minOverlap;
			}
		}
	}
	for (auto& axis : axes2) {
		Vec proj1 = projectOnAxis(vertices1, axis);
		Vec proj2 = projectOnAxis(vertices2, axis);

		float overlap = getOverlapLength(proj1, proj2);
		if (overlap < NORMAL_TOLERANCE) { // shapes are not overlapping
			return {};
		} else {
			if (overlap < minOverlap) {
				minOverlap = overlap;
				t_mtv.x = axis.x * minOverlap;
				t_mtv.y = axis.y * minOverlap;
				// ideally we would do this only once for the minimal overlap
				// but this is very cheap operation
			}
		}
	}

	// need to reverse MTV if center offset and overlap are not pointing in the same direction
	bool notPointingInTheSameDirection = dotProduct(getCenter(obb1) - getCenter(obb2), t_mtv) < 0.0f;
	if (notPointingInTheSameDirection) {
		t_mtv.x = -t_mtv.x;
		t_mtv.y = -t_mtv.y;
	}
	return t_mtv;
}

bool Shape::SAT(Shape const& other) {
	auto t_mtv = Vec{};
	auto& vertices1 = vertices;
	auto vertices2 = getVertices(other);

	auto& axes1 = normals;
	auto& axes2 = other.normals;

	// we need to find the minimal overlap and axis on which it happens
	auto minOverlap = std::numeric_limits<float>::max();

	for (auto& axis : axes1) {
		auto proj1 = projectOnAxis(vertices1, axis);
		auto proj2 = projectOnAxis(vertices2, axis);

		auto overlap = getOverlapLength(proj1, proj2);
		if (overlap == 0.f) { // shapes are not overlapping
			return false;
		} else {
			if (overlap < minOverlap) {
				minOverlap = overlap;
				t_mtv.x = axis.x * minOverlap;
				t_mtv.y = axis.y * minOverlap;
			}
		}
	}
	for (auto& axis : axes2) {
		auto proj1 = projectOnAxis(vertices1, axis);
		auto proj2 = projectOnAxis(vertices2, axis);

		auto overlap = getOverlapLength(proj1, proj2);
		if (overlap == 0.f) { // shapes are not overlapping
			return false;
		} else {
			if (overlap < minOverlap) {
				minOverlap = overlap;
				t_mtv.x = axis.x * minOverlap;
				t_mtv.y = axis.y * minOverlap;
				// ideally we would do this only once for the minimal overlap
				// but this is very cheap operation
			}
		}
	}
	return true;
}

bool Shape::overlaps(Shape const& other) const {
	bool ret{true};
	if (vertices.at(0).x > other.vertices.at(1).x) { ret = false; }
	if (vertices.at(1).x < other.vertices.at(0).x) { ret = false; }
	if (vertices.at(0).y > other.vertices.at(2).y) { ret = false; }
	if (vertices.at(2).y < other.vertices.at(0).y) { ret = false; }
	return ret;
}

bool Shape::contains_point(Vec point) { 
	bool ret{true};
	if (vertices.at(0).x > point.x) { ret = false; }
	if (vertices.at(1).x < point.x) { ret = false; }
	if (vertices.at(0).y > point.y) { ret = false; }
	if (vertices.at(2).y < point.y) { ret = false; }
	return ret;
}

void Shape::render(sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (num_sides == 4) {// define a 100x100 square, red, with a 10x10 texture mapped on it
		sf::Vertex vertices[] = {sf::Vertex(vertices[0].position - cam, sf::Color::Red, sf::Vector2f(0, 0)), sf::Vertex(vertices[1].position - cam, sf::Color::Red, sf::Vector2f(0, 10)),
								 sf::Vertex(vertices[2].position - cam, sf::Color::Red, sf::Vector2f(10, 10)), sf::Vertex(vertices[3].position - cam, sf::Color::Red, sf::Vector2f(10, 10))};
		// draw it
		win.draw(vertices, 4, sf::Quads);
	}
}

float Shape::get_height_at(float x) const {
	auto rise = vertices.at(1).y - vertices.at(0).y;
	auto run = vertices.at(1).x - vertices.at(0).x;
	auto slope = -1 * rise / run;
	auto max_height = std::max(vertices.at(2).y - vertices.at(1).y, vertices.at(3).y - vertices.at(0).y);
	auto min_height = std::min(vertices.at(2).y - vertices.at(1).y, vertices.at(3).y - vertices.at(0).y);
	// y intercept is always the left ramp height
	auto b = vertices.at(3).y - vertices.at(0).y;
	auto y = std::min(slope * x + b, max_height);
	return y;
}

bool Shape::AABB_handle_left_collision_static(Shape const& immovable) {
	bool colliding = false;
	// check that the shape is an initialized quad
	if (vertices.size() < 4) {
		return false;
	} else {
		// left collision
		if (AABB_is_left_collision(immovable)) {
			position.x = immovable.vertices[1].x + 1;
			update();
			colliding = true;
		}
	}

	return colliding;
}

bool Shape::AABB_handle_right_collision_static(Shape const& immovable) {
	bool colliding = false;
	// chack that the shape is an initialized quad
	if (vertices.size() < 4) {
		return false;
	} else {
		// right collision
		if (AABB_is_right_collision(immovable)) {
			position.x = immovable.vertices[0].x - dimensions.x - 1;
			update();
			colliding = true;
		}
	}
	return colliding;
}

float const small_value = 0.001f;
float const epsilon = 0.0005f;

bool Shape::AABB_is_left_collision(Shape const& immovable) {
	if (immovable.vertices.size() < 4) {
		return false;
	} else {
		return (position.x - epsilon < immovable.vertices[1].x && (position.y + dimensions.y) > vertices[1].y + small_value && position.y < vertices[2].y - small_value && position.x + dimensions.x > immovable.vertices[1].x);
	}
}

bool Shape::AABB_is_right_collision(Shape const& immovable) {
	if (vertices.size() < 4) {
		return false;
	} else {
		return (position.x + dimensions.x + epsilon > immovable.vertices[0].x && (position.y + dimensions.y) > vertices[1].y + small_value && position.y < vertices[2].y - small_value && position.x < immovable.vertices[0].x);
	}
}

} // namespace shape
