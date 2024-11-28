
#include "Background.hpp"
#include "../service/ServiceProvider.hpp"
#include "../setup/EnumLookups.hpp"
#include <algorithm>
#include <imgui.h>

namespace bg {

int const tile_dim{256};

Background::Background(automa::ServiceProvider& svc, int bg_id) : labels{{0, "dusk"}, {5, "night"}, {3, "rosy_haze"}, {18, "woods"}} {
	auto type = labels.contains(bg_id) ? labels.at(bg_id) : "black";
	auto const& in_data = svc.data.background[type];
	dimensions.x = in_data["dimensions"][0].as<int>();
	dimensions.y = in_data["dimensions"][1].as<int>();
	scroll_pane = dimensions;
	auto index{0};
	for (auto& layer : in_data["layers"].array_view()) {
		layers.push_back({index, layer["scroll_speed"].as<float>(), layer["parallax"].as<float>()});
		layers.back().physics.set_global_friction(1.f);
		layers.back().sprite.setTexture(svc.assets.get_background(bg_id));
		layers.back().sprite.setTextureRect(sf::IntRect{{0, dimensions.y * index}, dimensions});
		++index;
	}
}

void Background::update(automa::ServiceProvider& svc, sf::Vector2<float> observed_camvel) {
	for (auto& layer : layers) {
		// backtrack sprites for infinite scroll effect
		if (layer.physics.position.x < -scroll_pane.x) { layer.physics.position.x = 0.f; }
		if (layer.physics.position.x > 0.f) { layer.physics.position.x = -scroll_pane.x; }
		if (layer.physics.position.y < -scroll_pane.y) { layer.physics.position.y = 0.f; }
		if (layer.physics.position.y > 0.f) { layer.physics.position.y = -scroll_pane.y; }
		layer.physics.velocity.x = layer.scroll_speed;
		layer.physics.update_euler(svc);
	}
}

void Background::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float>& campos, sf::Vector2<float>& mapdim) {
	auto epsilon = 0.999f;
	for (auto& layer : layers) {
		auto final_position = layer.physics.position - campos * layer.parallax;
		for (auto i{0}; i < 2; ++i) {
			for (auto j{0}; j < 2; ++j) {
				layer.sprite.setPosition(final_position + sf::Vector2<float>{static_cast<float>(dimensions.x * epsilon) * static_cast<float>(i), static_cast<float>(dimensions.y * epsilon) * static_cast<float>(j)});
				win.draw(layer.sprite);
			}
		}
	}
}

void Background::debug() {
	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = 1.0;
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	ImGuiViewport const* viewport = ImGui::GetMainViewport();
	ImVec2 work_pos = viewport->WorkPos;
	ImVec2 work_size = viewport->WorkSize;
	ImVec2 window_pos, window_pos_pivot;
	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	window_flags |= ImGuiWindowFlags_NoMove;
	ImGui::SetNextWindowBgAlpha(0.65f);
	if (ImGui::Begin("Background Parameters", b_debug, window_flags)) {
		auto index{0};
		for (auto& layer : layers) {
			ImGui::Separator();
			ImGui::Text("Layer %i", index);
			std::string plabel = "parallax" + std::to_string(index);
			std::string slabel = "speed" + std::to_string(index);
			ImGui::SliderFloat(plabel.data(), &layer.parallax, 0.f, 1.f);
			ImGui::SliderFloat(slabel.data(), &layer.scroll_speed, -8.f, 8.f);
			++index;
		}
	}
	ImGui::End();
}

} // namespace bg
