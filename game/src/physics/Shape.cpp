
#include <fornani/graphics/Colors.hpp>
#include <fornani/physics/Shape.hpp>
#include <fornani/utils/Constants.hpp>
#include <fornani/utils/Math.hpp>
#include <cmath>

namespace fornani::shape {

Shape::Shape(sf::Vector2f dim, int num_vertices) {
	vertices.reserve(num_vertices);
	vertices.push_back({});
	vertices.push_back({dim.x, 0.f});
	vertices.push_back({dim.x, dim.y});
	if (num_vertices > 3) { vertices.push_back({0.f, dim.y}); }
}

void Shape::set_dimensions(sf::Vector2f const new_dim) {
	if (vertices.size() < 4) { return; }
	vertices[1].x = vertices[0].x + new_dim.x;
	vertices[2].x = vertices[0].x + new_dim.x;
	vertices[2].y = vertices[0].y + new_dim.y;
	vertices[3].y = vertices[0].y + new_dim.y;
}

sf::Vector2f Shape::perp(sf::Vector2f edg) const {
	auto x = -edg.y;
	auto y = edg.x;
	auto mag = std::sqrt(x * x + y * y);
	if (mag > 0.0f) {
		float inv = 1.0f / mag;
		x *= inv;
		y *= inv;
	}
	return {x, y};
}

void Shape::set_position(sf::Vector2f const new_pos) {
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

sf::Vector2f Shape::get_normalized(sf::Vector2f const v) const {
	float length = util::magnitude(v);
	if (length == 0.f) { return sf::Vector2f(); }
	return sf::Vector2f(v.x / length, v.y / length);
}

sf::Vector2f Shape::get_normal(sf::Vector2f const v) { return sf::Vector2f(-v.y, v.x); }

sf::Vector2f Shape::project_on_axis(std::vector<sf::Vector2f> const& vertices, sf::Vector2f const& axis) const {
	float min = vertices[0].x * axis.x + vertices[0].y * axis.y;
	float max = min;

	for (size_t i = 1; i < vertices.size(); ++i) {
		float projection = vertices[i].x * axis.x + vertices[i].y * axis.y;
		if (projection < min) min = projection;
		if (projection > max) max = projection;
	}

	return {min, max};
}

sf::Vector2f Shape::project_circle_on_axis(sf::Vector2f center, float radius, sf::Vector2f const axis) const {
	float projection = dot_product(center, axis);
	return sf::Vector2f(projection - radius, projection + radius);
}

sf::Vector2f Shape::get_top() const { return {(vertices[0].x + vertices[1].x) * 0.5f, (vertices[0].y + vertices[1].y) * 0.5f}; }

std::vector<sf::Vector2f> Shape::get_vertices(Shape const& shape) { return shape.vertices; }

std::vector<sf::Vector2f> Shape::get_poles(sf::CircleShape const& circle) {
	auto ret = std::vector<sf::Vector2f>{};
	auto normals = get_normals();
	for (auto& normal : normals) {
		auto r1 = circle.getPosition() + normal * circle.getRadius();
		auto r2 = circle.getPosition() - normal * circle.getRadius();
		ret.push_back(r1);
		ret.push_back(r2);
	}
	return ret;
}

sf::Vector2f Shape::get_MTV(Shape const& obb1, Shape const& obb2) {
	auto t_mtv = sf::Vector2f{};
	auto const& vertices1 = vertices;
	auto const vertices2 = get_vertices(obb2);

	auto const& axes1 = get_normals();
	auto const& axes2 = obb2.get_normals();

	// we need to find the minimal overlap and axis on which it happens
	auto minOverlap = std::numeric_limits<float>::max();

	for (auto& axis : axes1) {
		sf::Vector2f proj1 = project_on_axis(vertices1, axis);
		sf::Vector2f proj2 = project_on_axis(vertices2, axis);

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
		sf::Vector2f proj1 = project_on_axis(vertices1, axis);
		sf::Vector2f proj2 = project_on_axis(vertices2, axis);

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
	auto t_mtv = sf::Vector2f{};
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

bool Shape::circle_SAT(sf::CircleShape const& circle) const {
	auto normals = get_normals();
	for (auto& axis : normals) {
		auto proj1 = project_on_axis(vertices, axis);
		auto proj2 = project_circle_on_axis(circle.getPosition(), circle.getRadius(), axis);
		if (!are_overlapping(proj1, proj2)) { return false; }
	}
	// check fourth axis
	auto closest_vertex_axis{sf::Vector2f{}};
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

sf::Vector2f Shape::circle_SAT_MTV(sf::CircleShape const& circle) const {
	auto ret = sf::Vector2f{};
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
	auto closest_vertex_axis{sf::Vector2f{}};
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

sf::Vector2f Shape::compute_mtv(sf::Vector2f p) {
	float min_overlap = std::numeric_limits<float>::max();
	sf::Vector2f best_axis{};

	auto axes = get_normals();

	for (auto& axis : axes) {
		auto proj_shape = project_on_axis(vertices, axis);
		auto proj_point = project_circle_on_axis(p, 4.f, axis);

		if (!are_overlapping(proj_shape, proj_point)) return {};

		float overlap = get_overlap_length(proj_shape, proj_point);

		if (overlap < min_overlap) {
			min_overlap = overlap;

			float shape_center = (proj_shape.x + proj_shape.y) * 0.5f;
			float point_proj = dot_product(p, axis);

			sf::Vector2f dir = axis;

			if (point_proj < shape_center) dir = -axis;

			best_axis = dir;
		}
	}

	return best_axis * min_overlap;
}

bool Shape::overlaps(Shape const& other) const {
	if (vertices.at(0).x > other.vertices.at(1).x) { return false; }
	if (vertices.at(1).x < other.vertices.at(0).x) { return false; }
	if (vertices.at(0).y > other.vertices.at(2).y) { return false; }
	if (vertices.at(2).y < other.vertices.at(0).y) { return false; }
	return true;
}

bool Shape::overlaps(sf::FloatRect const& other) const {
	if (vertices.at(0).x > other.position.x + other.size.x) { return false; }
	if (vertices.at(1).x < other.position.x) { return false; }
	if (vertices.at(0).y > other.position.y + other.size.y) { return false; }
	if (vertices.at(2).y < other.position.y) { return false; }
	return true;
}

bool Shape::overlaps_circle(sf::Vector2f center, float radius) const {

	sf::Vector2f pos = vertices[0];
	sf::Vector2f size = get_dimensions();

	float left = pos.x;
	float right = pos.x + size.x;
	float top = pos.y;
	float bottom = pos.y + size.y;

	auto closest = sf::Vector2f{std::clamp(center.x, left, right), std::clamp(center.y, top, bottom)};

	float dx = center.x - closest.x;
	float dy = center.y - closest.y;

	return (dx * dx + dy * dy) <= (radius * radius);
}

bool Shape::overlaps(sf::Vector2f point) const {
	if (vertices.at(0).x > point.x) { return false; }
	if (vertices.at(1).x < point.x) { return false; }
	if (vertices.at(0).y > point.y) { return false; }
	if (vertices.at(2).y < point.y) { return false; }
	return true;
}

bool Shape::contains_point(sf::Vector2f point) {
	bool ret{true};
	if (vertices.at(0).x > point.x) { ret = false; }
	if (vertices.at(1).x < point.x) { ret = false; }
	if (vertices.at(0).y > point.y) { ret = false; }
	if (vertices.at(2).y < point.y) { ret = false; }
	return ret;
}

void Shape::render(sf::RenderWindow& win, sf::Vector2f cam, sf::Color color) {
	if (vertices.size() == 4) {
		auto mcolor = non_square() ? sf::Color{0, 0, 255, 24} : color;
		sf::Vertex line1[] = {{vertices[0] - cam, mcolor}, {vertices[1] - cam, mcolor}, {vertices[2] - cam, mcolor}};
		win.draw(line1, 3, sf::PrimitiveType::Triangles);
		sf::Vertex line2[] = {{vertices[0] - cam, mcolor}, {vertices[2] - cam, mcolor}, {vertices[3] - cam, mcolor}};
		win.draw(line2, 3, sf::PrimitiveType::Triangles);
	}
}

void Shape::draw(sf::RenderTexture& tex) {
	std::uint8_t alpha = 212;
	if (vertices.size() == 3) {
		sf::Vertex line[] = {{vertices[0], sf::Color{255, 255, 0, 100}}, {vertices[1], sf::Color{255, 255, 0, 100}}, {vertices[2], sf::Color{255, 255, 0, 100}}};
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

std::vector<sf::Vector2f> Shape::get_normals() const {
	size_t n = vertices.size();
	std::vector<sf::Vector2f> ret(n);

	for (size_t i = 0; i < n; ++i) {
		auto const& current = vertices[i];
		auto const& next = vertices[(i + 1) % n];

		auto edge = next - current;
		ret[i] = perp(edge);
	}

	return ret;
}

std::vector<sf::Vector2f> Shape::get_edges() const {
	std::vector<sf::Vector2f> ret{};
	for (auto i{0}; i < vertices.size(); ++i) { ret.push_back({vertices[static_cast<std::size_t>(i + 1) % vertices.size()].x - vertices[i].x, vertices[static_cast<std::size_t>(i + 1) % vertices.size()].y - vertices[i].y}); }
	return ret;
}

auto Shape::get_sloped_vertex(bool lower) const -> sf::Vector2f {
	auto const count = vertices.size();
	if (count < 2) { return {}; }

	std::size_t first = 0;
	std::size_t second = 1;
	auto longest = (vertices[1] - vertices[0]).lengthSquared();

	for (std::size_t i = 0; i < count; ++i) {
		for (std::size_t j = i + 1; j < count; ++j) {
			auto const length = (vertices[j] - vertices[i]).lengthSquared();
			if (length > longest) {
				longest = length;
				first = i;
				second = j;
			}
		}
	}

	if (lower) { return vertices[first].y > vertices[second].y ? vertices[first] : vertices[second]; }
	return vertices[first].y < vertices[second].y ? vertices[first] : vertices[second];
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
	return std::clamp(y, 0.f, std::abs(max_height));
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
		return (get_position().x - epsilon < immovable.vertices[1].x && (get_position().y + get_dimensions().y) > vertices[1].y + small_value && get_position().y < vertices[2].y - small_value &&
				get_position().x + get_dimensions().x > immovable.vertices[1].x);
	}
}

bool Shape::AABB_is_right_collision(Shape const& immovable) {
	if (vertices.size() < 4) {
		return false;
	} else {
		return (get_position().x + get_dimensions().x + epsilon > immovable.vertices[0].x && (get_position().y + get_dimensions().y) > vertices[1].y + small_value && get_position().y < vertices[2].y - small_value &&
				get_position().x < immovable.vertices[0].x);
	}
}

} // namespace fornani::shape
