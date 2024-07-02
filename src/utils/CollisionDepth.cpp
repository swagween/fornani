#include "CollisionDepth.hpp"
#include "CollisionDepth.hpp"
#include "CollisionDepth.hpp"
#include "Shape.hpp"
#include <iostream>

namespace util {

void CollisionDepth::calculate(shape::Shape const& native, shape::Shape const& other) {
	if (other.top() < native.bottom() && other.bottom() > native.top() && other.get_center().x < native.get_center().x) { candidate.left = other.right() - native.left(); }
	if (other.top() < native.bottom() && other.bottom() > native.top() && other.get_center().x > native.get_center().x) { candidate.right = other.left() - native.right(); }
	if (other.left() < native.right() && other.right() > native.left() && other.get_center().y < native.get_center().y) { candidate.top = other.bottom() - native.top(); }
	if (other.left() < native.right() && other.right() > native.left() && other.get_center().y > native.get_center().y) { candidate.bottom = other.top() - native.bottom(); }

	try_push();
}

void CollisionDepth::update() {
	if (stream.empty()) { return; }
	while (stream.size() > stream_size) { stream.pop_front(); }
	out_depth = {};
	for (auto& depth : stream) {
		out_depth.left += depth.left;
		out_depth.right += depth.right;
		out_depth.top += depth.top;
		out_depth.bottom += depth.bottom;
	}
	out_depth.left /= static_cast<float>(stream.size());
	out_depth.right /= static_cast<float>(stream.size());
	out_depth.top /= static_cast<float>(stream.size());
	out_depth.bottom /= static_cast<float>(stream.size());
}

void CollisionDepth::print() {
	std::cout << "Stream size: " << stream.size() << "\n";
	std::cout << ">>>\n";
	std::cout << "-Out Depth-\n";
	std::cout << "Left..: " << out_depth.left << "\n";
	std::cout << "Right.: " << out_depth.right << "\n";
	std::cout << "Top...: " << out_depth.top << "\n";
	std::cout << "Bottom: " << out_depth.bottom << "\n";
	std::cout << ">>>\n\n\n";
	return;
	std::cout << ">>>\n";
	std::cout << "-Stream-\n";
	auto ctr{0};
	for (auto& depth : stream) {
		std::cout << "Slot " << ctr << ":\n";
		std::cout << "Left..: " << depth.left << "\n";
		std::cout << "Right.: " << depth.right << "\n";
		std::cout << "Top...: " << depth.top << "\n";
		std::cout << "Bottom: " << depth.bottom << "\n";
		++ctr;
	}
}

void CollisionDepth::render(shape::Shape const& bounding_box, sf::RenderWindow& win, sf::Vector2<float> cam) {
	collision_ray.setFillColor(sf::Color::Cyan);

	// left
	collision_ray.setSize({out_depth.left, 2.f});
	collision_ray.setPosition(bounding_box.left() - cam.x, bounding_box.top() + bounding_box.dimensions.y * 0.5f - cam.y);
	win.draw(collision_ray);
	// right
	collision_ray.setSize({out_depth.right, 2.f});
	collision_ray.setPosition(bounding_box.right() - cam.x, bounding_box.top() + bounding_box.dimensions.y * 0.5f - cam.y);
	win.draw(collision_ray);
	// top
	collision_ray.setSize({2.f, out_depth.top});
	collision_ray.setPosition(bounding_box.left() - cam.x + bounding_box.dimensions.x * 0.5f, bounding_box.top() - cam.y);
	win.draw(collision_ray);
	// bottom
	collision_ray.setSize({2.f, out_depth.bottom});
	collision_ray.setPosition(bounding_box.left() - cam.x + bounding_box.dimensions.x * 0.5f, bounding_box.bottom() - cam.y);
	win.draw(collision_ray);
}

void CollisionDepth::try_push() {
	if (!stream.empty()) {
		// don't allow for large delta
		if (abs(abs(stream.back().left) - abs(candidate.left)) > depth_throwaway) { candidate.left = stream.back().left; }
		if (abs(abs(stream.back().right) - abs(candidate.right)) > depth_throwaway) { candidate.right = stream.back().right; }
		if (abs(abs(stream.back().top) - abs(candidate.top)) > depth_throwaway) { candidate.top = stream.back().top; }
		if (abs(abs(stream.back().bottom) - abs(candidate.bottom)) > depth_throwaway) { candidate.bottom = stream.back().bottom; }
		//if (abs(stream.back().top - candidate.top) > depth_throwaway) { return; }
		//if (abs(stream.back().bottom - candidate.bottom) > depth_throwaway) { return; }
		// ignore negative depths (positive for left and top)
		//if (candidate.left < crush_threshold) { candidate.left = 0.f; }
		//if (candidate.right > -crush_threshold) { candidate.right = 0.f; }
		if (candidate.top < crush_threshold) { candidate.top = 0.f; }
		if (candidate.bottom > -crush_threshold) { candidate.bottom = 0.f; }
	}

	stream.push_back(candidate);
}

} // namespace util
