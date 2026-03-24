
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>
#include <fornani/world/Train.hpp>

namespace fornani::world {

Train::Train(automa::ServiceProvider& svc, sf::Vector2i platform_position, int style)
	: Animatable{svc, "train", {401, 128}}, vfx::Chain{svc, vfx::SpringParameters{0.96f, 0.4f, 0.f, 0.f}, {}, 8}, m_wheels{svc, "train_wheels", {303, 39}}, m_platform_position{platform_position}, m_style{style}, m_prepare_arrive{200},
	  m_prepare_leave{200}, m_play_horn{500} {
	center();
	m_wheels.center();
	for (auto [i, link] : std::views::enumerate(links)) {
		auto index = i == 0 ? 0 : i == 2 ? 1 + random::random_range(0, 1) : 3 + random::random_range(0, 4);
		auto position = sf::Vector2f{750.f * static_cast<float>(i - 2) + 340.f, 30.f};
		m_cars.push_back(TrainCar{index, position});
	}
	Chain::flags.set(vfx::ChainFlags::ignore_player_collision);
	auto f_target = sf::Vector2f{m_platform_position} * constants::f_cell_size;
	Chain::set_position(sf::Vector2f{-3000.f, f_target.y});
	m_steering.physics.position = sf::Vector2f{-3000.f, f_target.y};
	set_flag(TrainFlags::away);
	m_wheels.push_and_set_animation("standard", {0, 8, 32, -1});
}

void Train::update(automa::ServiceProvider& svc, Map& map, player::Player& player) {

	m_prepare_arrive.update();
	m_prepare_leave.update();
	m_play_horn.update();

	if (!has_flag_set(TrainFlags::stopped)) { m_wheels.tick(); }

	auto incoming = has_flag_set(TrainFlags::approaching) || has_flag_set(TrainFlags::stopped);
	auto f_target = sf::Vector2f{m_platform_position} * constants::f_cell_size;
	auto actual_target = (incoming) ? f_target : sf::Vector2f{-2000.f, f_target.y};
	if (incoming) {
		m_steering.seek(actual_target, 0.0002f);
	} else if (has_flag_set(TrainFlags::leaving)) {
		m_steering.physics.acceleration.x = 0.1f;
		m_steering.physics.simple_update();
	}
	auto closest_car = m_steering.physics.position;
	for (auto [i, link] : std::views::enumerate(links)) {
		auto car_pos = link.get_bob() - m_cars[i].position;
		if ((player.get_center() - car_pos).lengthSquared() < (player.get_center() - closest_car).lengthSquared()) { closest_car = car_pos; }
	}

	auto wheel_speed = links.back().variables.anchor_physics.actual_speed();
	m_wheels.set_framerate(64 - static_cast<int>(54.f * std::clamp(wheel_speed, 0.f, 0.9f)));

	if (has_flag_set(TrainFlags::approaching) || has_flag_set(TrainFlags::leaving) || m_prepare_arrive.running()) {
		svc.soundboard.repeat_sound("train_moving", 1, closest_car);
		if (svc.ticker.every_second()) {
			if (random::percent_chance(30)) {
				m_play_horn.start();
				m_horn_selection = random::coin_flip() ? 0 : 1;
			}
		}
	}
	if (m_play_horn.running()) {
		auto horn = links[0].get_bob() - m_cars[0].position;
		auto pitch = std::clamp(horn.x - player.get_ear_position().x, 0.f, 40.f);
		auto to_pitch = horn.x > player.get_ear_position().x ? 1.f - (pitch / 1000.f) : 1.f;
		m_horn_selection == 0 ? svc.soundboard.repeat_sound("train_horn_1", 2, horn, to_pitch) : svc.soundboard.repeat_sound("train_horn_2", 2, horn, to_pitch);
	}

	// arrive
	if (links.size() < 2) { return; }
	if (has_flag_set(TrainFlags::approaching) && (links.at(1).get_anchor() - f_target).length() < 16.f) {
		m_prepare_arrive.start();
		set_flag(TrainFlags::approaching, false);
	}
	if (!has_flag_set(TrainFlags::stopped) && m_prepare_arrive.is_almost_complete()) {
		svc.soundboard.play_sound("train_steam", closest_car);
		set_flag(TrainFlags::stopped);
	}

	// leave
	if (has_flag_set(TrainFlags::leaving) && m_steering.physics.position.x > map.real_dimensions.x + 7000.f && !has_flag_set(TrainFlags::away)) {
		set_flag(TrainFlags::away);
		auto f_target = sf::Vector2f{m_platform_position} * constants::f_cell_size;
		Chain::set_position(sf::Vector2f{-3000.f, f_target.y});
		m_steering.physics.position = sf::Vector2f{-3000.f, f_target.y};
	}

	Chain::set_position(m_steering.physics.position);
	Chain::update(svc, map, player, 0.99f);
	tick();
	snap_to_axis(false);
	auto arrive = svc.world_clock.get_hours() % 2 == 1;
	auto leave = svc.world_clock.get_hours() % 2 == 0;
	if (arrive && has_flag_set(TrainFlags::away)) {
		set_flag(TrainFlags::approaching);
		set_flag(TrainFlags::away, false);
	}
	if (leave && has_flag_set(TrainFlags::stopped)) {
		m_prepare_leave.start();
		svc.soundboard.play_sound("train_steam", closest_car);
		set_flag(TrainFlags::stopped, false);
	}
	if (m_prepare_leave.is_almost_complete()) {
		set_flag(TrainFlags::leaving);
		set_flag(TrainFlags::stopped, false);
	}
}

void Train::render(sf::RenderWindow& win, sf::Vector2f cam) {
	bool debug_draw = false;
	set_channel(m_style);
	m_wheels.set_channel(m_style);
	if (has_flag_set(TrainFlags::away)) { return; }
	if (m_cars.size() != links.size()) { return; }
	sf::CircleShape debug_circle{};
	debug_circle.setRadius(4.f);
	for (auto [i, link] : std::views::enumerate(links)) {
		set_frame(m_cars[i].index);
		Animatable::set_position(link.get_bob() - cam - m_cars[i].position);
		win.draw(*this);
		m_wheels.set_position(link.get_bob() - cam - m_cars[i].position + sf::Vector2f{-24.f, 89.f});
		win.draw(m_wheels);

		debug_circle.setFillColor(colors::blue);
		debug_circle.setPosition(link.get_bob() - cam);
		if (debug_draw) { win.draw(debug_circle); }
		debug_circle.setFillColor(colors::goldenrod);
		debug_circle.setPosition(link.get_bob() - cam - m_cars[i].position);
		if (debug_draw) { win.draw(debug_circle); }
	}
	debug_circle.setFillColor(colors::red);
	debug_circle.setPosition(m_steering.physics.position - cam);
	if (debug_draw) { win.draw(debug_circle); }
	if (debug_draw) { debug(); }
}

void Train::debug() {
	static auto sz = ImVec2{180.f, 250.f};
	ImGui::SetNextWindowSize(sz);
	if (ImGui::Begin("Train Debug")) {
		ImGui::Text("Away..........: %s", has_flag_set(TrainFlags::away) ? "Yes" : "");
		ImGui::Text("Approaching...: %s", has_flag_set(TrainFlags::approaching) ? "Yes" : "");
		ImGui::Text("Stopped.......: %s", has_flag_set(TrainFlags::stopped) ? "Yes" : "");
		ImGui::Text("Leaving.......: %s", has_flag_set(TrainFlags::leaving) ? "Yes" : "");
		ImGui::Separator();
		ImGui::Text("Prepare Arrive: %i", m_prepare_arrive.get());
		ImGui::Text("Horn Sound: %i", m_play_horn.get());
		ImGui::Text("Link Speed: %.4f", links.back().variables.anchor_physics.actual_speed());
		ImGui::End();
	}
}

} // namespace fornani::world
