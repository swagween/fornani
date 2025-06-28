
#include "editor/canvas/entity/Enemy.hpp"

namespace pi {

Enemy::Enemy() : Entity("enemies") { repeatable = true; }

Enemy::Enemy(int id) : Entity("enemies", id, {1, 1}) { repeatable = true; }

std::unique_ptr<Entity> Enemy::clone() const { return std::make_unique<Enemy>(*this); }

void Enemy::serialize(dj::Json& out) { Entity::serialize(out); }

void Enemy::unserialize(dj::Json const& in) { Entity::unserialize(in); }

void Enemy::expose() { Entity::expose(); }

void Enemy::render(sf::RenderWindow& win, sf::Vector2<float> cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{250, 250, 80, 60}) : drawbox.setFillColor(sf::Color::Transparent);
	Entity::render(win, cam, size);
}

} // namespace pi
