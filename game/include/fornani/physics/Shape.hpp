
#pragma once

#include <SFML/Graphics.hpp>

#include <vector>

namespace fornani::shape {

class Shape {

  public:
	explicit Shape(sf::Vector2f dim = {32.f, 32.f}, int num_vertices = 4);

	void set_position(sf::Vector2f const new_pos);
	void set_dimensions(sf::Vector2f const new_dim);

	sf::Vector2f perp(sf::Vector2f edg) const;

	sf::Vector2f get_normalized(sf::Vector2f const v) const;
	sf::Vector2f get_normal(sf::Vector2f const v);
	sf::Vector2f project_on_axis(std::vector<sf::Vector2f> const& vertices, sf::Vector2f const& axis) const;
	sf::Vector2f project_circle_on_axis(sf::Vector2f center, float radius, sf::Vector2f const axis) const;
	sf::Vector2f get_top() const;
	std::vector<sf::Vector2f> get_vertices(Shape const& shape);
	std::vector<sf::Vector2f> get_poles(sf::CircleShape const& circle);
	sf::Vector2f get_MTV(Shape const& obb1, Shape const& obb2);
	bool SAT(Shape const& other);
	bool circle_SAT(sf::CircleShape const& circle) const;
	sf::Vector2f circle_SAT_MTV(sf::CircleShape const& circle) const;
	sf::Vector2f compute_mtv(sf::Vector2f test);
	bool overlaps(Shape const& other) const;
	bool overlaps(sf::FloatRect const& other) const;
	bool overlaps(sf::Vector2f point) const;
	bool overlaps_circle(sf::Vector2f center, float radius) const;
	bool contains_point(sf::Vector2f point);
	void draw(sf::RenderTexture& tex);
	void render(sf::RenderWindow& win, sf::Vector2f cam, sf::Color color = sf::Color{0, 255, 255, 48});
	std::vector<sf::Vector2f> get_normals() const;
	std::vector<sf::Vector2f> get_edges() const;

	[[nodiscard]] auto as_rect() const -> sf::FloatRect { return sf::FloatRect{vertices[0], get_dimensions()}; }
	[[nodiscard]] auto dot_product(sf::Vector2f const a, sf::Vector2f const b) const -> float { return a.x * b.x + a.y * b.y; }
	[[nodiscard]] auto are_overlapping(sf::Vector2f const a, sf::Vector2f const b) const -> bool { return a.x <= b.y && a.y >= b.x; }
	[[nodiscard]] auto get_overlap_length(sf::Vector2f const a, sf::Vector2f const b) const -> float { return are_overlapping(a, b) ? std::min(a.y, b.y) - std::max(a.x, b.x) : 0.f; }
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

	std::vector<sf::Vector2f> vertices{};

  private:
	sf::Vector2f position;
	int tile_id{};
};

} // namespace fornani::shape
