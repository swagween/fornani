
#include "Shape.hpp"

namespace shape {

Shape::Vec Shape::perp(Vec edg) {
	Vec temp = Vec(-edg.y, edg.x);
	double mag;
	double a_squared = temp.x * temp.x;
	double b_squared = temp.y * temp.y;
	mag = sqrt(a_squared + b_squared);
	if (abs(mag) > 0) {
		temp.x = temp.x / mag;
		temp.y = temp.y / mag;
	}
	return temp;
}

Shape::Shape() {
	// assume square
	for (int i = 0; i < 4; i++) {
		vertices.push_back(Vec(0.0f, 0.0f));
		edges.push_back(Vec(0.0f, 0.0f));
		normals.push_back(Vec(0.0f, 0.0f));
	}
	num_sides = 4;
	dimensions = sf::Vector2<float>(32.0f, 32.0f);
	init();
}

Shape::Shape(std::vector<Vec> verts, std::vector<Vec> edg) {
	vertices.clear();
	edges.clear();
	for (int i = 0; i < 4; i++) {
		vertices.push_back(verts[i]);
		edges.push_back(edg[i]);
		normals.push_back(perp(edg[i]));
	}
	position = vertices[0];
	num_sides = (int)verts.size();
	init();
}

Shape::Shape(Vec dim) {
	for (int i = 0; i < 4; i++) {
		edges.push_back(Vec(0.0f, 0.0f));
		normals.push_back(Vec(0, 0));
	}
	vertices.push_back(Vec(0.0f, 0.0f));
	vertices.push_back(Vec(dim.x, 0.0f));
	vertices.push_back(Vec(dim.x, dim.y));
	vertices.push_back(Vec(0.0f, dim.y));
	dimensions = dim;
	num_sides = 4;
	init();
}

void Shape::init() {
	for (int i = 0; i < vertices.size(); i++) {
		edges[i].x = vertices[(i + 1) % vertices.size()].x - vertices[i].x;
		edges[i].y = vertices[(i + 1) % vertices.size()].y - vertices[i].y;
		normals[i] = perp(edges[i]);
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
	Vec result = Vec(0, 0);
	if (!vertices.empty()) {
		int x1 = vertices.front().x;
		int y1 = vertices.front().y;
		int x2 = vertices.back().x;
		int y2 = vertices.back().y;
		result.x = (x1 + x2) / 2;
		result.y = (y1 + y2) / 2;
	}
	return result;
}

static float const NORMAL_TOLERANCE = 0.0001f;

float Shape::getLength(const Vec v) { return std::sqrt(v.x * v.x + v.y * v.y); }

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

Shape::Vec Shape::getPerpendicularAxis(const std::vector<Vec> vertices, std::size_t index) {
	assert(index >= 0 && index < 4); // rect has 4 possible axes
	return getNormal(getNormalized(vertices.at(index)));
}

// axes for which we'll test stuff. Two for each box, because testing for parallel axes isn't needed

std::array<Shape::Vec, 8> Shape::getPerpendicularAxes(const std::vector<Vec> vertices1, std::vector<Vec> const& vertices2) {
	std::array<Vec, 8> axes;

	axes[0] = getPerpendicularAxis(vertices1, 0);
	axes[1] = getPerpendicularAxis(vertices1, 1);
	axes[2] = getPerpendicularAxis(vertices1, 2);
	axes[3] = getPerpendicularAxis(vertices1, 3);

	axes[4] = getPerpendicularAxis(vertices2, 0);
	axes[5] = getPerpendicularAxis(vertices2, 1);
	axes[6] = getPerpendicularAxis(vertices2, 2);
	axes[7] = getPerpendicularAxis(vertices2, 3);
	return axes;
}

// Separating Axis Theorem (SAT) collision test
// Minimum Translation Vector (MTV) is returned for the first Oriented Bounding Box (OBB)
bool Shape::testCollision(Shape const& obb1, Shape const& obb2, Vec& mtv) {
	std::vector<Vec> vertices1 = getVertices(obb1);
	std::vector<Vec> vertices2 = getVertices(obb2);

	std::array<Vec, 4> axes1;
	std::array<Vec, 4> axes2;

	axes1[0] = obb1.normals.at(0);
	axes1[1] = obb1.normals.at(1);
	axes1[2] = obb1.normals.at(2);
	axes1[3] = obb1.normals.at(3);

	axes2[0] = obb2.normals.at(0);
	axes2[1] = obb2.normals.at(1);
	axes2[2] = obb2.normals.at(2);
	axes2[3] = obb2.normals.at(3);

	for (auto& axis : axes1) {
		Vec proj1 = projectOnAxis(vertices1, axis);
		Vec proj2 = projectOnAxis(vertices2, axis);

		float overlap = getOverlapLength(proj1, proj2);
		if (overlap == 0.f) { // shapes are not overlapping
			return false;
		}
	}
	for (auto& axis : axes2) {
		Vec proj1 = projectOnAxis(vertices1, axis);
		Vec proj2 = projectOnAxis(vertices2, axis);

		float overlap = getOverlapLength(proj1, proj2);
		if (overlap == 0.f) { // shapes are not overlapping
			return false;
		}
	}
	return true;
}

Shape::Vec Shape::testCollisionGetMTV(Shape const& obb1, Shape const& obb2) {
	Vec t_mtv = Vec(0.0f, 0.0f);
	std::vector<Vec> vertices1 = getVertices(obb1);
	std::vector<Vec> vertices2 = getVertices(obb2);

	std::array<Vec, 4> axes1;
	std::array<Vec, 4> axes2;

	axes1[0] = obb1.normals.at(0);
	axes1[1] = obb1.normals.at(1);
	axes1[2] = obb1.normals.at(2);
	axes1[3] = obb1.normals.at(3);

	axes2[0] = obb2.normals.at(0);
	axes2[1] = obb2.normals.at(1);
	axes2[2] = obb2.normals.at(2);
	axes2[3] = obb2.normals.at(3);

	// we need to find the minimal overlap and axis on which it happens
	float minOverlap = std::numeric_limits<float>::infinity();

	for (auto& axis : axes1) {
		Vec proj1 = projectOnAxis(vertices1, axis);
		Vec proj2 = projectOnAxis(vertices2, axis);

		float overlap = getOverlapLength(proj1, proj2);
		if (overlap < NORMAL_TOLERANCE) { // shapes are not overlapping
			return Vec(0.0f, 0.0f);
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
			return Vec(0.0f, 0.0f);
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
	Vec t_mtv = Vec(0.0f, 0.0f);
	std::vector<Vec> vertices1 = getVertices(*this);
	std::vector<Vec> vertices2 = getVertices(other);

	std::array<Vec, 4> axes1;
	std::array<Vec, 4> axes2;

	axes1[0] = this->normals.at(0);
	axes1[1] = this->normals.at(1);
	axes1[2] = this->normals.at(2);
	axes1[3] = this->normals.at(3);

	axes2[0] = other.normals.at(0);
	axes2[1] = other.normals.at(1);
	axes2[2] = other.normals.at(2);
	axes2[3] = other.normals.at(3);

	// we need to find the minimal overlap and axis on which it happens
	float minOverlap = std::numeric_limits<float>::infinity();

	for (auto& axis : axes1) {
		Vec proj1 = projectOnAxis(vertices1, axis);
		Vec proj2 = projectOnAxis(vertices2, axis);

		float overlap = getOverlapLength(proj1, proj2);
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
		Vec proj1 = projectOnAxis(vertices1, axis);
		Vec proj2 = projectOnAxis(vertices2, axis);

		float overlap = getOverlapLength(proj1, proj2);
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

bool Shape::overlaps(Shape const& other) {
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
