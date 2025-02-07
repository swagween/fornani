
#include "fornani/utils/Shape.hpp"
#include <ccmath/math/power/sqrt.hpp>
#include "fornani/utils/Math.hpp"
#include <iostream>

namespace fornani::shape {

Shape::Shape(Vec dim, int num_vertices) {
	vertices.reserve(num_vertices);
	vertices.push_back({});
	vertices.push_back({dim.x, 0.f});
	vertices.push_back({dim.x, dim.y});
	if (num_vertices > 3) { vertices.push_back({0.f, dim.y}); }
}

void Shape::set_dimensions(Vec const new_dim) {
	if (vertices.size() < 4) { return; }
	vertices[1].x = vertices[0].x + new_dim.x;
	vertices[2].x = vertices[0].x + new_dim.x;
	vertices[2].y = vertices[0].y + new_dim.y;
	vertices[3].y = vertices[0].y + new_dim.y;
}

sf::Vector2<float> Shape::perp(Vec edg) const {
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

void Shape::set_position(Vec const new_pos) {
	position = new_pos;
	auto dimensions = get_dimensions();
	if (vertices.size() >= 4) {
		vertices[0] = new_pos;
		vertices[1].x = new_pos.x + dimensions.x;
		vertices[1].y = new_pos.y;
		vertices[2].x = new_pos.x + dimensions.x;
		vertices[2].y = new_pos.y + dimensions.y;
		vertices[3].x = new_pos.x;
		vertices[3].y = new_pos.y + dimensions.y;
	} else {
		for (auto& vert : vertices) { vert += new_pos; }
	}
}

// Returns normalized vector
Shape::Vec Shape::get_normalized(Vec const v) {
	float length = util::magnitude(v);
	if (length == 0.f) { return Vec(); }
	return Vec(v.x / length, v.y / length);
}

// Returns right hand perpendicular vector
Shape::Vec Shape::get_normal(const Vec v) { return Vec(-v.y, v.x); }

// Find minimum and maximum projections of each vertex on the axis
Shape::Vec Shape::project_on_axis(const std::vector<Vec> vertices, const Vec axis) {
	float min = std::numeric_limits<float>::infinity();
	float max = -std::numeric_limits<float>::infinity();
	for (auto& vertex : vertices) {
		float projection = dot_product(vertex, axis);
		if (projection < min) { min = projection; }
		if (projection > max) { max = projection; }
	}
	return Vec(min, max);
}

Shape::Vec Shape::project_circle_on_axis(Vec center, float radius, Vec const axis) {
	float projection = dot_product(center, axis);
	return Vec(projection - radius, projection + radius);
}

std::vector<Shape::Vec> Shape::get_vertices(Shape const& shape) { return shape.vertices; }

std::vector<sf::Vector2<float>> Shape::get_poles(sf::CircleShape const& circle) {
	auto ret = std::vector<sf::Vector2<float>>{};
	auto normals = get_normals();
	for (auto& normal : normals) {
		auto r1 = circle.getPosition() + normal * circle.getRadius();
		auto r2 = circle.getPosition() - normal * circle.getRadius();
		ret.push_back(r1);
		ret.push_back(r2);
	}
	return ret;
}

Shape::Vec Shape::get_MTV(Shape const& obb1, Shape const& obb2) {
	auto t_mtv = Vec{};
	auto const& vertices1 = vertices;
	auto const vertices2 = get_vertices(obb2);

	auto const& axes1 = get_normals();
	auto const& axes2 = obb2.get_normals();

	// we need to find the minimal overlap and axis on which it happens
	auto minOverlap = std::numeric_limits<float>::max();

	for (auto& axis : axes1) {
		Vec proj1 = project_on_axis(vertices1, axis);
		Vec proj2 = project_on_axis(vertices2, axis);

		float overlap = get_overlap_length(proj1, proj2);
		if (overlap == 0.f) { // shapes are not overlapping
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
		Vec proj1 = project_on_axis(vertices1, axis);
		Vec proj2 = project_on_axis(vertices2, axis);

		float overlap = get_overlap_length(proj1, proj2);
		if (overlap == 0.f) { // shapes are not overlapping
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
	bool notPointingInTheSameDirection = dot_product(obb1.get_center() - obb2.get_center(), t_mtv) < 0.0f;
	if (notPointingInTheSameDirection) {
		t_mtv.x = -t_mtv.x;
		t_mtv.y = -t_mtv.y;
	}
	return t_mtv;
}

bool Shape::SAT(Shape const& other) {
	auto t_mtv = Vec{};
	auto const& vertices1 = vertices;
	auto const vertices2 = get_vertices(other);

	auto const& axes1 = get_normals();
	auto const& axes2 = other.get_normals();

	// we need to find the minimal overlap and axis on which it happens
	auto minOverlap = std::numeric_limits<float>::max();

	for (auto& axis : axes1) {
		auto proj1 = project_on_axis(vertices1, axis);
		auto proj2 = project_on_axis(vertices2, axis);

		auto overlap = get_overlap_length(proj1, proj2);
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
		auto proj1 = project_on_axis(vertices1, axis);
		auto proj2 = project_on_axis(vertices2, axis);

		auto overlap = get_overlap_length(proj1, proj2);
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

bool Shape::circle_SAT(sf::CircleShape const& circle) {
	auto normals = get_normals();
	for (auto& axis : normals) {
		auto proj1 = project_on_axis(vertices, axis);
		auto proj2 = project_circle_on_axis(circle.getPosition(), circle.getRadius(), axis);
		if (!are_overlapping(proj1, proj2)) { return false; }
	}
	//check fourth axis
	auto closest_vertex_axis{sf::Vector2<float>{}};
	auto distance{std::numeric_limits<float>::max()};
	auto min_dist{std::numeric_limits<float>::max()};
	for (auto& vertex : vertices) {
		distance = util::magnitude(vertex - circle.getPosition());
		if (distance < min_dist) {
			closest_vertex_axis = vertex - circle.getPosition();
			min_dist = distance;
		}
	}
	closest_vertex_axis = get_normalized(closest_vertex_axis);
	auto proj1 = project_on_axis(vertices, closest_vertex_axis);
	auto proj2 = project_circle_on_axis(circle.getPosition(), circle.getRadius(), closest_vertex_axis);
	if (!are_overlapping(proj1, proj2)) { return false; }
	return true;
}

sf::Vector2<float> Shape::circle_SAT_MTV(sf::CircleShape const& circle) {
	auto ret = sf::Vector2<float>{};
	auto min_overlap = std::numeric_limits<float>::max();
	auto normals = get_normals();
	for (auto& axis : normals) {
		auto proj1 = project_on_axis(vertices, axis);
		auto proj2 = project_circle_on_axis(circle.getPosition(), circle.getRadius(), axis);
		auto overlap = get_overlap_length(proj1, proj2);
		if (overlap < min_overlap) {
			min_overlap = overlap;
			ret = axis * min_overlap;
		}
		if (!are_overlapping(proj1, proj2)) { return {}; }
	}
	// check fourth axis
	auto closest_vertex_axis{sf::Vector2<float>{}};
	auto distance{std::numeric_limits<float>::max()};
	auto min_dist{std::numeric_limits<float>::max()};
	for (auto& vertex : vertices) {
		distance = util::magnitude(vertex - circle.getPosition());
		if (distance < min_dist) {
			closest_vertex_axis = vertex - circle.getPosition();
			min_dist = distance;
		}
	}
	closest_vertex_axis = get_normalized(closest_vertex_axis);
	auto proj1 = project_on_axis(vertices, closest_vertex_axis);
	auto proj2 = project_circle_on_axis(circle.getPosition(), circle.getRadius(), closest_vertex_axis);
	auto overlap = get_overlap_length(proj1, proj2);
	if (overlap < min_overlap) {
		min_overlap = overlap;
		ret = closest_vertex_axis * min_overlap;
	}
	if (!are_overlapping(proj1, proj2)) { return {}; }
	return ret;
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
	if (vertices.size() == 4) {
		auto color = non_square() ? sf::Color{0, 0, 255, 48} : sf::Color{0, 255, 255, 48};
		sf::Vertex line1[] = {{vertices[0] - cam, color}, {vertices[1] - cam, color}, {vertices[2] - cam, color}};
		win.draw(line1, 3, sf::PrimitiveType::Triangles);
		sf::Vertex line2[] = {{vertices[0] - cam, color}, {vertices[2] - cam, color}, {vertices[3] - cam, color}};
		win.draw(line2, 3, sf::PrimitiveType::Triangles);
	}
}

void Shape::draw(sf::RenderTexture& tex) {
	uint8_t alpha = 212;
	if (vertices.size() == 3) {
		sf::Vertex line[] = {{vertices[0], sf::Color{255, 255, 0, 100}},
							 {vertices[1], sf::Color{255, 255, 0, 100}},
							 {vertices[2], sf::Color{255, 255, 0, 100}}};
		tex.draw(line, 3, sf::PrimitiveType::Triangles);
	}
	if (vertices.size() == 4) {
		auto color = non_square() ? sf::Color{80, 80, 255, alpha} : sf::Color{0, 255, 255, alpha};
		sf::Vertex line1[] = {{vertices[0], color}, {vertices[1], color}, {vertices[2], color}};
		tex.draw(line1, 3, sf::PrimitiveType::Triangles);
		sf::Vertex line2[] = {{vertices[0], color}, {vertices[2], color}, {vertices[3], color}};
		tex.draw(line2, 3, sf::PrimitiveType::Triangles);
	}
	for (int i{0}; i < vertices.size(); ++i) {
		if (!non_square()) { break; }
		auto edges = get_edges();
		auto start = vertices[i] + edges[i] * 0.5f;
		auto scale = -8.f;
		auto normals = get_normals();
		sf::Vertex norm[] = {{start, sf::Color{255, 0, 0, alpha}}, {{start.x + normals[i].x * scale, start.y + normals[i].y * scale}, sf::Color{255, 0, 0, alpha}}};
		tex.draw(norm, 2, sf::PrimitiveType::Lines);
	}
}

std::vector<sf::Vector2<float>> Shape::get_normals() const {
	std::vector<sf::Vector2<float>> ret{};
	auto edges = get_edges();
	for (auto i{0}; i < vertices.size(); ++i) {
		ret.push_back(perp(edges[i]));
	}
	return ret;
}

std::vector<sf::Vector2<float>> Shape::get_edges() const {
	std::vector<sf::Vector2<float>> ret{};
	for (auto i{0}; i < vertices.size(); ++i) { ret.push_back({vertices[static_cast<size_t>(i + 1) % vertices.size()].x - vertices[i].x, vertices[static_cast<size_t>(i + 1) % vertices.size()].y - vertices[i].y}); }
	return ret;
}

float Shape::get_height_at(float x) const {
	if (!non_square()) { return 0.f; }
	auto rise = vertices.at(0).y - vertices.at(1).y;
	auto run = vertices.at(1).x - vertices.at(0).x;
	auto sign = vertices.at(1).y > vertices.at(0).y ? -1.f : 1.f;
	if (run == 0.f) { return 0.f; }
	auto slope = rise / run;
	auto max_height = vertices.size() == 4 ? std::max(vertices.at(2).y - vertices.at(1).y, vertices.at(3).y - vertices.at(0).y) : std::max(vertices.at(2).y - vertices.at(0).y, vertices.at(2).y - vertices.at(1).y);
	auto min_height = vertices.size() == 4 ? std::min(vertices.at(2).y - vertices.at(1).y, vertices.at(3).y - vertices.at(0).y) : std::min(vertices.at(2).y - vertices.at(0).y, vertices.at(2).y - vertices.at(1).y);
	// y intercept is always the left ramp height
	auto b = vertices.size() == 4 ? vertices.at(3).y - vertices.at(0).y : vertices.at(2).y - vertices.at(0).y;
	auto y = slope * x + b;
	return std::clamp(y, 0.f, abs(max_height));
}

float Shape::get_radial_factor() const {
	auto a = vertices.at(1).x - vertices.at(0).x;
	auto b = vertices.at(1).y - vertices.at(0).y;
	if (a == 0) { return 1.f; }
	auto c_squared = a * a + b * b;
	auto c = std::sqrt(c_squared);
	return a / c;
}

bool Shape::AABB_handle_left_collision_static(Shape const& immovable) {
	bool colliding = false;
	// check that the shape is an initialized quad
	if (vertices.size() < 4) {
		return false;
	} else {
		// left collision
		if (AABB_is_left_collision(immovable)) {
			set_position({immovable.vertices[1].x + 1, get_position().y});
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
			set_position({immovable.vertices[0].x - get_dimensions().x - 1, get_position().y});
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
		return (get_position().x - epsilon < immovable.vertices[1].x && (get_position().y + get_dimensions().y) > vertices[1].y + small_value && get_position().y < vertices[2].y - small_value && get_position().x + get_dimensions().x > immovable.vertices[1].x);
	}
}

bool Shape::AABB_is_right_collision(Shape const& immovable) {
	if (vertices.size() < 4) {
		return false;
	} else {
		return (get_position().x + get_dimensions().x + epsilon > immovable.vertices[0].x && (get_position().y + get_dimensions().y) > vertices[1].y + small_value && get_position().y < vertices[2].y - small_value && get_position().x < immovable.vertices[0].x);
	}
}

} // namespace shape
