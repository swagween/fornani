

#include "fornani/entities/atmosphere/Atmosphere.hpp"
#include <imgui.h>
#include "fornani/entities/player/Player.hpp"
#include "fornani/level/Map.hpp"
#include "fornani/service/ServiceProvider.hpp"

#include <tracy/Tracy.hpp>

#include "fornani/utils/Random.hpp"

namespace fornani::vfx {

Atmosphere::Atmosphere(automa::ServiceProvider& svc, sf::Vector2<float> span, int type) {
	auto density{32};
	auto const chunks = (span.x / (svc.constants.cell_size * 16.f)) * (span.y / (svc.constants.cell_size * 16.f));
	for (auto i{0}; i < density * chunks; ++i) {
		auto const startx = util::Random::random_range_float(0.f, span.x);
		auto const starty = util::Random::random_range_float(0.f, span.y);
		fireflies.push_back(std::make_unique<Firefly>(svc, sf::Vector2{startx, starty}));
	}
	if (type == 1) {
		density = 4;
		for (auto i{0}; i < density * chunks; ++i) {
			auto startx = util::Random::random_range_float(0.f, span.x);
			auto starty = util::Random::random_range_float(0.f, span.y);
			dragonflies.push_back(Dragonfly(svc, {startx, starty}));
		}
	}
}

void Atmosphere::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	for (auto const& fly : fireflies) { fly->update(svc, map); }
	for (auto& fly : dragonflies) { fly.update(svc, map, player); }
}

void Atmosphere::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	ZoneScopedN("Atmospher::render");
	svc.out_value = 0;
	for (auto const& fly : fireflies) { fly->render(svc, win, cam); }
	for (auto& fly : dragonflies) { fly.render(svc, win, cam); }
	if (svc.ticker.every_x_frames(60)) { svc.logger.add_log(std::to_string(svc.out_value).c_str()); }
}

void Atmosphere::debug() {
	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = 1.0;
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	ImGui::SetNextWindowPos({10.f, 200.f}, ImGuiCond_Always);
	window_flags |= ImGuiWindowFlags_NoMove;
	ImGui::SetNextWindowBgAlpha(0.65f);
	if (ImGui::Begin("Firefly Parameters", nullptr, window_flags)) {
		ImGui::SliderFloat("Seek", &forces.seek, 0.f, 0.01f, "%.6f");
		ImGui::SliderFloat("Walk", &forces.walk, 0.f, 0.1f, "%.5f");
		ImGui::SliderFloat("Evade", &forces.evade, 0.f, 1.f, "%.5f");
	}
	ImGui::End();
	for (auto& fly : dragonflies) { fly.set_forces(forces.seek, forces.walk, forces.evade); }
}

} // namespace fornani::vfx
