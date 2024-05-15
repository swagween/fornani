#include "Platform.hpp"
#include <cmath>
#include "../entities/player/Player.hpp"
#include "../service/ServiceProvider.hpp"

namespace world {

Platform::Platform(automa::ServiceProvider& svc, sf::Vector2<float> position, sf::Vector2<float> dimensions, float extent, std::string_view specifications, float start_point)
	: shape::Collider(dimensions, position), path_position(start_point) {

	auto const& in_data = svc.data.platform[specifications];
	if (in_data["sticky"].as_bool()) { flags.attributes.set(PlatformAttributes::sticky); }
	if (in_data["loop"].as_bool()) { flags.attributes.set(PlatformAttributes::loop); }
	if (in_data["repeating"].as_bool()) { flags.attributes.set(PlatformAttributes::repeating); }

	for (auto& point : in_data["track"].array_view()) { track.push_back({position.x + (point[0].as<float>() * extent), position.y + (point[1].as<float>() * extent)}); }
	track_shape.setPointCount(track.size());
	if (flags.attributes.test(PlatformAttributes::loop)) { track.push_back(track.at(0)); }

	for (std::size_t x = 0; x < track.size() - 1; ++x) {
		path_length += compute_length(track[x + 1] - track[x]);
		track_shape.setPoint(x, track[x] + dimensions * 0.5f);
	}

	track_shape.setOutlineColor(sf::Color(135, 132, 149, 140));
	track_shape.setFillColor(sf::Color(135, 132, 149, 140));
	track_shape.setOutlineThickness(2);

	physics.position = track_shape.getPoint(0);
	direction.lr = dir::LR::right;
	direction.und = dir::UND::down;
	counter.start();

	// set visuals
	style = 1;
	animation.set_params({0, 4, 16, -1});
	sprite.setTexture(svc.assets.t_platforms);
	auto scaled_dim = dimensions / svc.constants.cell_size;
	if (scaled_dim.x == 1) { offset = {0, 0}; }
	if (scaled_dim.x == 2) { offset = {32, 0}; }
	if (scaled_dim.x == 3) { offset = {0, 32}; }
	if (scaled_dim.y == 2) { offset = {0, 64}; }
	if (scaled_dim.y == 3) { offset = {0, 128}; }
}

void Platform::update(automa::ServiceProvider& svc, player::Player& player) {
	auto skip_value{16.f};
	auto edge_start = 0.f;
	player.collider.handle_collider_collision(bounding_box);
	for (std::size_t x = 0; x < track.size() - 1; ++x) {
		auto start = track[x];
		auto end = track[x + 1];
		auto len = compute_length(end - start);
		auto edge_end = edge_start + (len / path_length);
		if (edge_end >= path_position) {
			auto old_position = physics.position;
			physics.position.x = std::lerp(start.x, end.x, (path_position - edge_start) / (edge_end - edge_start));
			physics.position.y = std::lerp(start.y, end.y, (path_position - edge_start) / (edge_end - edge_start));
			if (player.collider.jumpbox.overlaps(bounding_box) && flags.attributes.test(PlatformAttributes::sticky)) {
				auto diff = physics.position - old_position;
				if (!(abs(diff.x) > skip_value || abs(diff.y) > skip_value)) { player.collider.physics.position += physics.position - old_position; }
			}
			break;
		} else {
			edge_start = edge_end;
		}
	}
	path_position += metrics.speed;
	if (path_position > 1.0f) { path_position = 0.f; }
	sync_components();

	counter.update();
	animation.update();
}

void Platform::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	track_shape.setPosition(-cam.x, -cam.y);
	sprite.setPosition(physics.position - cam);
	auto u = style * 96;
	auto v = animation.get_frame() * 224;
	auto lookup = sf::Vector2<int>{u, v} + offset;
	sprite.setTextureRect(sf::IntRect(sf::Vector2<int>(lookup), sf::Vector2<int>(dimensions)));
	win.draw(track_shape);
	win.draw(sprite);
	// Collider::render(win, cam);
}

float Platform::compute_length(sf::Vector2<float> const v) { return std::sqrt(v.x * v.x + v.y * v.y); }

} // namespace world
