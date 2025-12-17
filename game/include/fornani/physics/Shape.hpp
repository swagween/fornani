
#pragma once

#include <SFML/Graphics.hpp>

#include <vector>

namespace fornani::shape {

class Shape {

  public:
	using Vec = sf::Vector2f;

	explicit Shape(Vec dim = {32.f, 32.f}, int num_vertices = 4);

	void set_position(Vec const new_pos);
	void set_dimensions(Vec const new_dim);

	Vec perp(Vec edg) const;

	Vec get_normalized(Vec const v) const;
	Vec get_normal(Vec const v);
	Vec project_on_axis(std::vector<Vec> const vertices, Vec const axis) const;
	Vec project_circle_on_axis(Vec center, float radius, Vec const axis) const;
	std::vector<Vec> get_vertices(Shape const& shape);
	std::vector<sf::Vector2f> get_poles(sf::CircleShape const& circle);
	Vec get_MTV(Shape const& obb1, Shape const& obb2);
	bool SAT(Shape const& other);
	bool circle_SAT(sf::CircleShape const& circle) const;
	sf::Vector2f circle_SAT_MTV(sf::CircleShape const& circle) const;
	bool overlaps(Shape const& other) const;
	bool overlaps(sf::FloatRect const& other) const;
	bool overlaps(sf::Vector2f point) const;
	bool contains_point(Vec point);
	void draw(sf::RenderTexture& tex);
	void render(sf::RenderWindow& win, sf::Vector2f cam, sf::Color color = sf::Color{0, 255, 255, 48});
	std::vector<Vec> get_normals() const;
	std::vector<Vec> get_edges() const;

	[[nodiscard]] auto as_rect() const -> sf::FloatRect { return sf::FloatRect{vertices[0], get_dimensions()}; }
	[[nodiscard]] auto dot_product(Vec const a, Vec const b) const -> float { return a.x * b.x + a.y * b.y; }
	[[nodiscard]] auto are_overlapping(Vec const a, Vec const b) const -> bool { return a.x <= b.y && a.y >= b.x; }
	[[nodiscard]] auto get_overlap_length(Vec const a, Vec const b) const -> float { return are_overlapping(a, b) ? std::min(a.y, b.y) - std::max(a.x, b.x) : 0.f; }
	[[nodiscard]] auto non_square() const -> bool {
		if (vertices.size() < 4) { return true; }
		return vertices[0].y != vertices[1].y || vertices[2].y != vertices[3].y;
	}
	[[nodiscard]] auto get_position() const -> sf::Vector2f { return position; }
	[[nodiscard]] auto get_dimensions() const -> sf::Vector2f { return non_square() ? sf::Vector2f{32.f, 32.f} : vertices[2] - vertices[0]; }
	[[nodiscard]] auto get_center() const -> sf::Vector2f { return get_position() + get_dimensions() * 0.5f; }
	[[nodiscard]] auto left() const -> float { return get_position().x; }
	[[nodiscard]] auto right() const -> float { return get_position().x + get_dimensions().x; }
	[[nodiscard]] auto top() const -> float { return get_position().y; }
	[[nodiscard]] auto bottom() const -> float { return get_position().y + get_dimensions().y; }
	[[nodiscard]] float get_height_at(float x) const;
	[[nodiscard]] float get_radial_factor() const;

	bool AABB_handle_left_collision_static(Shape const& immovable);
	bool AABB_handle_right_collision_static(Shape const& immovable);
	bool AABB_is_left_collision(Shape const& immovable);
	bool AABB_is_right_collision(Shape const& immovable);

	std::vector<Vec> vertices{};

  private:
	sf::Vector2f position;
	int tile_id{};
};

} // namespace fornani::shape
