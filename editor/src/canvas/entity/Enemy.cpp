
#include "editor/canvas/entity/Enemy.hpp"

namespace pi {

Enemy::Enemy(fornani::automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "enemies") {
	unserialize(in);
	repeatable = true;
}

Enemy::Enemy(fornani::automa::ServiceProvider& svc, int id, int variant) : Entity(svc, "enemies", id, {1, 1}), m_variant{variant} { repeatable = true; }

std::unique_ptr<Entity> Enemy::clone() const { return std::make_unique<Enemy>(*this); }

void Enemy::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["variant"] = m_variant;
}

void Enemy::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	m_variant = in["variant"].as<int>();
}

void Enemy::expose() { Entity::expose(); }

void Enemy::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{250, 250, 80, 60}) : drawbox.setFillColor(sf::Color::Transparent);
	Entity::render(win, cam, size);
}

} // namespace pi
