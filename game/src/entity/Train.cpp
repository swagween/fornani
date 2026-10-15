
#include <fornani/entities/player/Player.hpp>
#include <fornani/entity/Train.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

Train::Train(automa::ServiceProvider& svc, dj::Json const& in) : Entity{svc, in, "train"}, m_prepare_arrive{200}, m_prepare_leave{200}, m_play_horn{500}, m_wheels{svc, "train_wheels", {303, 39}}, m_services{&svc} {
	unserialize(in);
	init(svc);
}

Train::Train(automa::ServiceProvider& svc) : Entity{svc, "train", 0}, m_prepare_arrive{200}, m_prepare_leave{200}, m_play_horn{500}, m_wheels{svc, "train_wheels", {303, 39}}, m_services{&svc} { init(svc); }

Train::Train(automa::ServiceProvider& svc, int style) : Entity{svc, "train", 0}, m_style{style}, m_prepare_arrive{200}, m_prepare_leave{200}, m_play_horn{500}, m_wheels{svc, "train_wheels", {303, 39}}, m_services{&svc} { init(svc); }

std::unique_ptr<Entity> Train::clone() const { return std::make_unique<Train>(*this); }

void Train::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["style"] = m_style;
}

void Train::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	m_style = in["style"].as<int>();
}

void Train::expose() { Entity::expose(); }

void Train::update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] SceneContext& context, [[maybe_unused]] player::Player& player) {

	if (!m_chain) { return; }

	m_prepare_arrive.update();
	m_prepare_leave.update();
	m_play_horn.update();

	if (!has_flag_set(TrainFlags::stopped)) {
		auto const wheel_speed = m_chain->links.back().variables.anchor_physics.actual_speed();
		auto const rate = 64 - static_cast<int>(54.f * std::clamp(wheel_speed, 0.f, 0.9f));
		m_wheels.set_framerate(rate);
		m_wheels.tick();
	}

	auto incoming = has_flag_set(TrainFlags::approaching) || has_flag_set(TrainFlags::stopped);
	auto f_target = get_world_position();
	auto actual_target = (incoming) ? f_target : sf::Vector2f{-2000.f, f_target.y};
	if (incoming) {
		m_steering.seek(actual_target, 0.0002f);
	} else if (has_flag_set(TrainFlags::leaving)) {
		m_steering.physics.acceleration.x = 0.1f;
		m_steering.physics.simple_update();
	}
	auto closest_car = m_steering.physics.position;
	for (auto [i, link] : std::views::enumerate(m_chain->links)) {
		auto car_pos = link.get_bob() - m_cars[i].position;
		if ((player.get_center() - car_pos).lengthSquared() < (player.get_center() - closest_car).lengthSquared()) { closest_car = car_pos; }
	}

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
		auto horn = m_chain->links[0].get_bob() - m_cars[0].position;
		auto pitch = std::clamp(horn.x - player.get_ear_position().x, 0.f, 40.f);
		auto to_pitch = horn.x > player.get_ear_position().x ? 1.f - (pitch / 1000.f) : 1.f;
		m_horn_selection == 0 ? svc.soundboard.repeat_sound("train_horn_1", 2, horn, to_pitch) : svc.soundboard.repeat_sound("train_horn_2", 2, horn, to_pitch);
	}

	// arrive
	if (m_chain->links.size() < 2) { return; }
	if (has_flag_set(TrainFlags::approaching) && (m_chain->links.at(1).get_anchor() - f_target).length() < 16.f) {
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
		auto f_target = get_world_position();
		m_chain->set_position(sf::Vector2f{-3000.f, f_target.y});
		m_steering.physics.position = sf::Vector2f{-3000.f, f_target.y};
	}

	m_chain->set_position(m_steering.physics.position);
	m_chain->update(svc, map, player, 0.99f);
	p_animatable.tick();
	m_chain->snap_to_axis(false);
	auto running = p_contingencies ? svc.quest_table.are_contingencies_met(*p_contingencies) : true;
	auto arrive = svc.world_clock.get_hours() % 2 == 1;
	auto leave = svc.world_clock.get_hours() % 2 == 0;
	if (arrive && has_flag_set(TrainFlags::away) && running) {
		set_flag(TrainFlags::approaching);
		set_flag(TrainFlags::away, false);
	}
	if (leave && has_flag_set(TrainFlags::stopped) && running) {
		m_prepare_leave.start();
		svc.soundboard.play_sound("train_steam", closest_car);
		set_flag(TrainFlags::stopped, false);
	}
	if (m_prepare_leave.is_almost_complete()) {
		set_flag(TrainFlags::leaving);
		set_flag(TrainFlags::stopped, false);
	}
}

void Train::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{60, 255, 120, 180}) : drawbox.setFillColor(sf::Color{60, 255, 120, 80});
	Entity::render(win, cam, size);
	if (m_editor) { return; }
	if (!m_chain) { return; }
	bool debug_draw = debug::is_debug();
	p_animatable.center();
	p_animatable.set_channel(m_style);
	m_wheels.set_channel(m_style);
	if (has_flag_set(TrainFlags::away)) { return; }
	if (m_cars.size() != m_chain->links.size()) { return; }
	sf::CircleShape debug_circle{};
	debug_circle.setRadius(4.f);
	for (auto [i, link] : std::views::enumerate(m_chain->links)) {
		p_animatable.set_frame(m_cars[i].index);
		p_animatable.set_position(link.get_bob() - cam - m_cars[i].position);
		win.draw(p_animatable);
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

void Train::render(sf::RenderWindow& win, graphics::DayNightShifter& shifter, sf::Vector2f cam) {
	highlighted ? drawbox.setFillColor(sf::Color{60, 255, 120, 180}) : drawbox.setFillColor(sf::Color{60, 255, 120, 80});
	Entity::render(win, cam, 1.f);
	if (m_editor) { return; }
	if (!m_chain) { return; }
	bool debug_draw = true;
	if (has_flag_set(TrainFlags::away)) { return; }
	if (m_cars.size() != m_chain->links.size()) { return; }
	p_animatable.center();
	p_animatable.set_channel(m_style);
	m_wheels.set_channel(m_style);
	sf::CircleShape debug_circle{};
	debug_circle.setRadius(4.f);
	for (auto [i, link] : std::views::enumerate(m_chain->links)) {
		p_animatable.set_frame(m_cars[i].index);
		m_wheels.set_position(link.get_bob() - cam - m_cars[i].position + sf::Vector2f{-24.f, 89.f});
		p_animatable.set_position(link.get_bob() - cam - m_cars[i].position);
		for (auto [j, sprite] : std::views::enumerate(m_sprites)) {
			if (!sprite) { continue; }
			sprite->center();
			sprite->set_position(p_animatable.get_window_position());
			sprite->set_frame(m_cars[i].index);
			shifter.render(*m_services, win, sprite->get_sprite(), j);
		}
		for (auto [j, sprite] : std::views::enumerate(m_wheel_sprites)) {
			if (!sprite) { continue; }
			sprite->center();
			sprite->set_position(m_wheels.get_window_position());
			sprite->set_frame(m_wheels.get_frame());
			sprite->set_position(m_wheels.get_window_position());
			shifter.render(*m_services, win, sprite->get_sprite(), j);
		}

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

void Train::init(automa::ServiceProvider& svc) {
	// m_textured = false;
	repeatable = false;
	auto const sprite_dim = sf::Vector2i{401, 128};
	p_animatable.set_dimensions(sprite_dim);
	m_chain.emplace(vfx::Chain(svc, vfx::SpringParameters{0.96f, 0.4f, 0.f, 0.f}, {}, 8));
	p_animatable.center();
	m_wheels.center();
	if (m_chain) {
		for (auto [i, link] : std::views::enumerate(m_chain->links)) {
			auto index = i == 0 ? 0 : i == 2 ? 1 : 2;
			auto position = sf::Vector2f{750.f * static_cast<float>(i - 2) + 340.f, 30.f};
			m_cars.push_back(TrainCar{index, position});
		}
		m_chain->flags.set(vfx::ChainFlags::ignore_player_collision);
		auto f_target = get_world_position();
		m_chain->set_position(sf::Vector2f{-3000.f, f_target.y});
		m_steering.physics.position = sf::Vector2f{-3000.f, f_target.y};
	}
	set_flag(TrainFlags::away);
	m_wheels.push_and_set_animation("0", {0, 8, 32, -1});
	m_wheels.push_animation("1", {8, 8, 32, -1});
	m_wheels.push_animation("2", {16, 8, 32, -1});
	p_animatable.push_and_set_animation("0", {0, 3, 32, -1});
	p_animatable.push_animation("1", {3, 3, 32, -1});
	p_animatable.push_animation("2", {6, 3, 32, -1});
	p_animatable.center();
	p_animatable.set_channel(m_style);
	m_wheels.set_channel(m_style);

	for (std::size_t i = 0; i < m_sprites.size(); ++i) {
		if (!m_sprites[i]) { m_sprites[i].emplace(p_animatable); }
		m_sprites[i]->set_animation(std::to_string(i));
	}
	for (std::size_t i = 0; i < m_wheel_sprites.size(); ++i) {
		if (!m_wheel_sprites[i]) { m_wheel_sprites[i].emplace(m_wheels); }
		m_wheel_sprites[i]->set_animation(std::to_string(i));
	}
}

void Train::debug() {
	if (!m_chain) { return; }
	static auto sz = ImVec2{180.f, 250.f};
	ImGui::SetNextWindowSize(sz);
	if (ImGui::Begin("Train Debug")) {
		ImGui::Text("Away..........: %s", has_flag_set(TrainFlags::away) ? "Yes" : "");
		ImGui::Text("Approaching...: %s", has_flag_set(TrainFlags::approaching) ? "Yes" : "");
		ImGui::Text("Stopped.......: %s", has_flag_set(TrainFlags::stopped) ? "Yes" : "");
		ImGui::Text("Leaving.......: %s", has_flag_set(TrainFlags::leaving) ? "Yes" : "");
		ImGui::Separator();
		ImGui::Text("Prepare Arrive:");
		ImGui::ProgressBar(m_prepare_arrive.get_normalized());
		ImGui::Text("Horn Sound: %i", m_play_horn.get());
		ImGui::Text("Link Speed: %.4f", m_chain->links.back().variables.anchor_physics.actual_speed());
		ImGui::End();
	}
}

} // namespace fornani
