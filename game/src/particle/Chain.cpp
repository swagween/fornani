
#include "fornani/particle/Chain.hpp"
#include <numbers>
#include <ranges>
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::vfx {

constexpr auto y_dampen_v = 0.3f;

Chain::Chain(automa::ServiceProvider& svc, SpringParameters params, sf::Vector2f position, int num_links, bool reversed, float spacing, bool linked) : m_root(position), m_external_dampen{0.07f} {
	if (!linked) {
		for (int i{0}; i < num_links; ++i) { links.push_back(Spring({params})); }
		m_grav = params.grav;
		int ctr{};
		auto sign = reversed ? -1.f : 1.f;
		for (auto& link : links) {
			if (ctr == 0) {
				link.set_anchor(position);
				link.lock();
			} else {
				link.cousin = &links.at(ctr - 1);
				if (link.cousin) { link.set_anchor(link.cousin.value()->get_bob()); }
			}
			link.set_bob(link.get_anchor() + sf::Vector2f{0.f, sign * spacing});
			++ctr;
		}
	} else {
		flags.set(ChainFlags::linked);
		for (int i{0}; i < num_links; ++i) { links.push_back(Spring({params})); }
		m_grav = params.grav;
		auto sign = reversed ? -1.f : 1.f;
		if (links.empty()) { return; }
		float angleStep = 2.f * std::numbers::pi / num_links;
		float radius = params.rest_length / (2.f * std::sin(angleStep * 0.5f));
		m_radius = radius;

		for (std::size_t i = 0; i < links.size(); ++i) {
			auto& link = links[i];
			float angle = angleStep * i;
			link.set_bob(position + sf::Vector2f{std::cos(angle) * radius, std::sin(angle) * radius});
		}

		m_centroid = position;

		for (std::size_t i = 1; i < links.size(); ++i) { links[i].cousin = &links[i - 1]; }
		links[0].cousin = &links.back();
	}
}

vfx::Chain::Chain(automa::ServiceProvider& svc, std::string_view tag, sf::Vector2i dim, SpringParameters params, sf::Vector2f position, int num_links, bool reversed, float spacing, bool linked)
	: Chain(svc, params, position, num_links, reversed, spacing, linked) {
	sprite.emplace(svc, tag, dim);
	sprite->center();
}

void Chain::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, float dampen) {
	bool const linked = flags.test(ChainFlags::linked);

	// ----------------------------------------
	// OPEN CHAIN (spring rope)
	// ----------------------------------------
	if (!linked) {

		auto external_force = sf::Vector2f{};
		auto ctr{0};
		for (auto& link : links) {
			if (ctr < links.size() - 1) { link.set_bob(links.at(static_cast<std::size_t>(ctr + 1)).get_anchor()); }
			if (!link.is_locked()) {
				if (link.cousin) { link.set_anchor(link.cousin.value()->get_bob()); }
			}
			if (link.sensor.within_bounds(player.get_collider().bounding_box)) {
				link.sensor.activate();
				external_force = {player.get_collider().physics.velocity.x * m_external_dampen * dampen, player.get_collider().physics.velocity.y * m_external_dampen * y_dampen_v * dampen};
			} else {
				link.sensor.deactivate();
			}
			link.update(svc, m_grav, external_force, !link.is_locked(), m_free ? true : ctr == links.size() - 1);
			++ctr;
		}

		return;
	}

	// ----------------------------------------
	// CLOSED RING (constraint system)
	// ----------------------------------------

	std::vector<sf::Vector2f> external_forces(links.size(), sf::Vector2f{});

	// Gather forces once
	for (std::size_t i = 0; i < links.size(); ++i) {
		auto& link = links[i];
		sf::Vector2f force{};

		if (link.sensor.within_bounds(player.get_collider().bounding_box)) {
			link.sensor.activate();

			force = player.get_collider().physics.velocity * dampen;

			if (flags.test(ChainFlags::ignore_player_collision)) force = {};
		} else {
			link.sensor.deactivate();
		}

		external_forces[i] = force;
	}

	// integrate particles
	for (std::size_t i = 0; i < links.size(); ++i) { links[i].update_constrained(svc, m_grav, external_forces[i]); }

	if (!flags.test(ChainFlags::no_collision)) {
		// collision
		m_avg_velocity = {};
		m_resistance = 0.85f;
		m_num_collisions = 0;
		for (auto& link : links) {
			sf::Vector2f p = link.get_bob();
			sf::Vector2f mtv{};
			mtv += map.compute_mtv(p);
			mtv += player.get_collider().bounding_box.compute_mtv(p);
			sf::Vector2f correction = mtv;
			m_avg_velocity += correction;
			link.set_bob(p + correction);
			if (mtv.length() > constants::tiny_value) { ++m_num_collisions; }

			// remove inward velocity (critical for slipperiness)
			sf::Vector2f n = mtv.normalized();
			float vn = util::dot(link.variables.bob_physics.velocity, n);

			if (vn < 0.f) { link.variables.bob_physics.velocity -= n * vn; }
		}
		m_avg_velocity /= static_cast<float>(links.size()) * -1.f;
	}

	// distance constraints
	constexpr int iterations = 8;
	if (!flags.test(ChainFlags::broken)) {
		for (int iter = 0; iter < iterations; ++iter) {
			// edge constraint
			for (std::size_t i = 0; i < links.size(); ++i) {
				std::size_t prev = (i + links.size() - 1) % links.size();

				auto& a = links[i];
				auto& b = links[prev];

				sf::Vector2f delta = a.get_bob() - b.get_bob();

				float distSq = delta.x * delta.x + delta.y * delta.y;
				if (distSq < 0.0001f) { continue; }

				float dist = std::sqrt(distSq);

				float error = dist - a.get_params().rest_length;

				sf::Vector2f correction = (delta / dist) * (error * 0.9f);

				a.set_bob(a.get_bob() - correction);
				b.set_bob(b.get_bob() + correction);
			}

			// radial constraint
			for (auto& a : links) {
				sf::Vector2f offset = a.get_bob() - *m_centroid;

				float radius = std::sqrt(offset.x * offset.x + offset.y * offset.y);
				if (radius < 0.0001f) { continue; }

				float error = radius - m_radius;
				float t = radius / m_radius; // 0 = center, 1 = correct radius

				float strength = error * 0.005f;

				// boost correction when too close to center
				if (t < 0.7f) {
					float boost = (1.f - t) * (1.f - t);
					strength *= (1.f + boost * 6.0f);
				}

				sf::Vector2f correction = (offset / radius) * strength;

				a.set_bob(a.get_bob() - correction);
			}
		}
	}
	// keep ring centered
	if (m_centroid && !flags.test(ChainFlags::broken)) {
		sf::Vector2f center{};
		for (auto& link : links) { center += link.get_bob(); }
		center /= static_cast<float>(links.size());
		sf::Vector2f offset = *m_centroid - center;
		for (auto& link : links) { link.set_bob(link.get_bob() + offset * 0.3f); }
	}
}

void Chain::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam, bool average) {
	for (auto [i, link] : std::views::enumerate(links)) {
		if (flags.test(ChainFlags::broken) && link.get_fade().is_complete()) { continue; }
		if (svc.greyblock_mode()) { link.render(win, cam); }
		if (sprite) {
			m_rotator.handle_rotation(*sprite, get_tangent(i), AnimatableAxis::frame, m_num_angles);
			sprite->set_channel(link.get_channel());
			auto pos = average ? link.get_average_bob_position() : link.get_bob();
			sprite->set_position(link.get_average_bob_position() - cam);
			win.draw(*sprite);
		}
	}
}

void Chain::set_position(sf::Vector2f to_position) {
	if (links.empty()) { return; }

	bool const linked = flags.test(ChainFlags::linked);

	if (linked) {
		m_centroid = to_position;
	} else {
		auto ctr{0};
		for (auto& link : links) {
			if (ctr == 0) {
				link.set_anchor(to_position);
				link.lock();
			} else {
				link.cousin = &links.at(ctr - 1);
			}
			++ctr;
		}
		return;
	}

	for (std::size_t i = 1; i < links.size(); ++i) { links[i].cousin = &links[i - 1]; }

	if (linked) { links[0].cousin = &links.back(); }
}

void Chain::set_end_position(sf::Vector2f to_position) {
	if (links.empty()) { return; }
	links.at(links.size() - 1).set_bob(to_position);
}

void Chain::lock_ends() {
	for (std::size_t i = 0; i < links.size(); ++i) {
		auto& link = links[i];
		if (i == 0 || i == links.size() - 1) { link.lock(); }
	}
}

void vfx::Chain::snap_to_axis(bool vert) {
	if (links.empty()) { return; }
	auto snap = vert ? links.at(0).get_anchor().x : links.at(0).get_anchor().y;
	for (auto& link : links) { vert ? link.set_anchor({snap, link.get_anchor().y}) : link.set_anchor({link.get_anchor().x, snap}); }
}

void Chain::simulate(automa::ServiceProvider& svc, int amount) {
	for (auto i = 0; i < amount; ++i) {
		for (std::size_t i = 0; i < links.size(); ++i) {
			auto& link = links[i];
			if (i < links.size() - 1) { link.set_bob(links.at(static_cast<std::size_t>(i + 1)).get_anchor()); }
			if (!link.is_locked()) {
				if (link.cousin) { link.set_anchor(link.cousin.value()->get_bob()); }
			}
			link.simulate(m_grav, !link.is_locked(), i == links.size() - 1);
		}
	}
}

void Chain::break_all() {
	flags.set(ChainFlags::broken);
	for (auto& link : links) {
		sf::Vector2f dir = (link.get_bob() - *m_centroid).normalized();
		link.variables.bob_physics.velocity += dir * 20.f;
		link.fade(random::random_range(80, 200));
	}
}

float Chain::compute_resistance() {
	float total = 0.f;
	for (auto& link : links) {
		sf::Vector2f v = link.variables.bob_physics.velocity; // or delta bob
		float speed = v.length();
		auto const& params = link.get_params();
		float stretch = std::abs((link.get_bob() - link.get_anchor()).length() - params.rest_length);
		total += speed * 0.5f + stretch * 1.5f;
	}
	return total / links.size();
}

sf::Vector2f Chain::get_tangent(std::size_t i) const {
	std::size_t prev = (i + links.size() - 1) % links.size();
	std::size_t next = (i + 1) % links.size();
	sf::Vector2f tangent = links[next].get_average_bob_position() - links[prev].get_average_bob_position();
	float lenSq = tangent.x * tangent.x + tangent.y * tangent.y;
	if (lenSq < 1e-6f) { return {}; }

	return tangent / std::sqrt(lenSq);
}

bool Chain::moving() const {
	if (links.empty()) { return false; }
	return std::abs(links.at(links.size() - 1).variables.bob_physics.velocity.y) > 0.001f;
}

auto Chain::get_percentage_colliding() const -> float { return static_cast<float>(m_num_collisions) / static_cast<float>(links.size()); }

auto Chain::contains_point(sf::Vector2f test) const -> bool {
	if (!flags.test(ChainFlags::linked)) { return false; }
	bool inside = false;
	for (std::size_t i = 0, j = links.size() - 1; i < links.size(); j = i++) {
		sf::Vector2f a = links[i].get_bob();
		sf::Vector2f b = links[j].get_bob();
		if (((a.y > test.y) != (b.y > test.y)) && (test.x < (b.x - a.x) * (test.y - a.y) / (b.y - a.y) + a.x)) { inside = !inside; }
	}
	return inside;
}

} // namespace fornani::vfx
