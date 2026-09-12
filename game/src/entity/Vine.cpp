
#include <fornani/core/Debug.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/entity/Vine.hpp>
#include <fornani/graphics/Renderer.hpp>
#include <fornani/graphics/SpriteRotator.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Math.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

constexpr auto simulations_v = 32;

Vine::Vine(automa::ServiceProvider& svc, int length, int size, bool foreground, bool reversed, std::vector<int> const platform_indeces)
	: Entity(svc, "vines", 0), m_length(length), m_chain(svc, {0.995f, 0.08f, static_cast<float>(size) * 0.5f, 14.f}, get_world_position(), length, reversed, 2.f), m_services(&svc), m_init{64} {
	for (auto const& i : platform_indeces) {
		if (i == -1) { continue; }
		add_platform(svc, i);
	}
	foreground ? m_flags.set(VineFlags::foreground) : m_flags.reset(VineFlags::foreground);
	m_init.start();
}

Vine::Vine(automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "vines"), m_services(&svc), m_chain(svc, {0.995f, 0.06f, 16.f, 14.f}, get_world_position(), in["length"].as<int>(), false, 2.f), m_init{64} {
	unserialize(in);
	m_json.emplace(in);
	m_init.start();
}

void Vine::init(automa::ServiceProvider& svc, world::Map& map) {
	m_map.emplace(&map);
	auto const& in_vine = svc.data.biomes["properties"][map.get_biome_string()]["vines"];
	p_animatable.set_texture(svc.assets.get_texture(in_vine["tag"].as_string()));
	p_animatable.set_dimensions({in_vine["dimensions"][0].as<int>(), in_vine["dimensions"][1].as<int>()});
	m_tapers = in_vine["tapers"].as<int>();
	m_dampen = in_vine["dampen"].as<float>();
	auto const variations = in_vine["variations"].as<int>();
	m_chain.parameters.gravity = in_vine["gravity"].as<float>();
	m_chain.set_rest_length(in_vine["rest_length"].as<float>());
	m_chain.set_spring_constant(in_vine["spring_constant"].as<float>());
	m_chain.set_dampen(in_vine["spring_dampen"].as<float>());
	m_angles = in_vine["angles"].as<int>();
	batch = true;
	p_animatable.center();
	auto index = util::Circuit(variations);
	auto last_index = random::random_range(0, variations - 1);
	auto ctr{0};
	for (auto& link : m_chain.links) {
		index.set(random::random_range(0, index.get_order()));
		if (index.get() == last_index) { index.modulate(1); }
		auto const sign = random::percent_chance(50) ? -1 : 1;
		encodings.push_back({index.get(), sign});
		last_index = index.get();
		// optionally add treasure container to vine segment
		if (random::percent_chance(5) && in_vine["treasure"].as_bool()) {
			auto rarity = item::Rarity::common;
			if (auto random_sample = random::random_range_float(0.0f, 1.0f); random_sample < constants.priceless) {
				rarity = item::Rarity::priceless;
			} else if (random_sample < constants.rare) {
				rarity = item::Rarity::rare;
			} else if (random_sample < constants.uncommon) {
				rarity = item::Rarity::uncommon;
			} else {
				rarity = item::Rarity::common;
			}
			m_treasure_balls.push_back(std::make_unique<entity::TreasureContainer>(*m_services, rarity, link.get_anchor(), ctr));
		}
		++ctr;
	}
	repeatable = false;
	m_chain.simulate(*m_services, 128);
	if (m_json) {
		for (auto& link : m_json.value()["mine"]["link_indeces"].as_array()) { add_mine(*m_services, link.as<int>()); }
	}
}

std::unique_ptr<Entity> Vine::clone() const { return std::make_unique<Vine>(*this); }

void Vine::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["foreground"] = is_foreground();
	out["length"] = m_length;
	for (auto const& plat : m_spawnable_platforms) { out["platform"]["link_indeces"].push_back(plat->get_index()); }
	for (auto const& mine : m_mine_data) { out["mine"]["link_indeces"].push_back(mine.index); }
}

void Vine::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	in["foreground"].as_bool() ? m_flags.set(VineFlags::foreground) : m_flags.reset(VineFlags::foreground);
	m_length = in["length"].as<int>();
	if (in["platform"]) {
		for (auto& link : in["platform"]["link_indeces"].as_array()) { add_platform(*m_services, link.as<int>()); }
	}
	if (in["mine"]) {
		for (auto& link : in["mine"]["link_indeces"].as_array()) { m_mine_data.push_back({link.as<int>()}); }
	}
}

void Vine::expose() {
	Entity::expose();
	static bool fg = is_foreground();
	ImGui::Checkbox("Foreground", &fg);
	ImGui::InputInt("Length", &m_length);
	for (auto [i, link] : std::views::enumerate(m_chain.links)) {
		ImGui::PushID(i);
		if (ImGui::SmallButton("+ plat")) { add_platform(*m_services, i); }
		ImGui::SameLine();
		if (ImGui::SmallButton("+ mine")) { m_mine_data.push_back(MineData{static_cast<int>(i)}); }
		ImGui::SameLine();
		if (ImGui::SmallButton("- plat")) { remove_platform(i); }
		ImGui::SameLine();
		if (ImGui::SmallButton("- mine")) {
			if (i < m_mine_data.size()) { m_mine_data.erase(m_mine_data.begin() + i); }
		}
		ImGui::SameLine();
		ImGui::Text("%i: ", static_cast<int>(i));
		for (auto const& plat : m_spawnable_platforms) {
			if (plat->get_index() == i) {
				ImGui::SameLine();
				ImGui::Text("Platform");
			}
		}
		for (auto const& mine : m_mine_data) {
			if (mine.index == i) {
				ImGui::SameLine();
				ImGui::Text("Mine");
			}
		}
		ImGui::PopID();
	}
	fg ? m_flags.set(VineFlags::foreground) : m_flags.reset(VineFlags::foreground);
}

void Vine::update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] SceneContext& context, [[maybe_unused]] player::Player& player) {
	Entity::update(svc, map, context, player);
	if (m_init.started()) { init(svc, map); }
	if (m_init.running()) {
		if (p_flags.test(EntityFlags::spawn_denied)) { m_spawnable_platforms.clear(); }
		for (int i = 0; i < simulations_v; ++i) { m_chain.update(svc, map, player); }
	} else {
		m_chain.update(svc, map, player, m_dampen);
	}
	m_init.update();
	for (auto const& ball : m_treasure_balls) { ball->update(svc, m_chain.links.at(ball->get_index()).get_bob()); }
	std::erase_if(m_treasure_balls, [](auto const& b) { return b->destroyed(); });
	for (auto const& mine : m_mines) {
		mine->update(svc, map, player);
		mine->set_position(m_chain.links.at(mine->get_index()).get_bob());
		m_chain.links.at(mine->get_index()).variables.bob_physics.apply_force({0.f, 10.f});
	}
	std::erase_if(m_mines, [](auto const& m) { return m->is_exploded(); });
	for (auto const& plat : m_spawnable_platforms) { plat->update(svc, player, m_chain.links.at(plat->get_index()).get_bob()); }
}

void Vine::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj, player::Player& player) {
	for (auto& ball : m_treasure_balls) { ball->on_hit(svc, map, proj, player); }
	for (auto& plat : m_spawnable_platforms) { plat->on_hit(svc, map, proj); }
	for (auto& mine : m_mines) {
		mine->on_hit(svc, map, proj, player);
		m_chain.links.at(mine->get_index()).variables.bob_physics.velocity = mine->get_collider().physics.velocity;
	}
}

void Vine::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{60, 255, 120, 180}) : drawbox.setFillColor(sf::Color{60, 255, 120, 80});
	Entity::render(win, cam, size);
	if (m_editor) { return; }
	if (debug::is_debug()) { m_chain.render(*m_services, win, cam); }
	for (auto const& ball : m_treasure_balls) { ball->render(*m_services, win, cam); }
	auto spro = vfx::SpriteRotator{};
	int ctr{0};
	auto current = 0.f;
	auto total = static_cast<float>(m_chain.links.size());
	if (debug::is_production()) {
		for (auto& link : m_chain.links) {
			auto rotation_angle = (link.get_bob() - link.get_anchor()).normalized();
			auto idx = 0;
			if (m_angles > 1) {
				spro.handle_rotation(p_animatable.get_sprite(), rotation_angle, m_angles);
				idx = spro.get_sprite_angle_index();
			}
			p_animatable.set_texture_rect(
				sf::IntRect({static_cast<int>((current / static_cast<float>(m_length)) * m_tapers * idx) * p_animatable.get_dimensions().x, encodings.at(ctr).at(0) * p_animatable.get_dimensions().y}, p_animatable.get_dimensions()));
			p_animatable.set_scale(sf::Vector2f{static_cast<float>(encodings.at(ctr).at(1)), 1.f} * constants::f_scale_factor);
			p_animatable.set_position(util::round_to_even(link.get_bob()) - cam);
			win.draw(p_animatable);
			++debug::draw_calls;
			++ctr;
			++current;
		}
	}
	for (auto const& plat : m_spawnable_platforms) { plat->render(win, cam); }
	for (auto const& mine : m_mines) { mine->render(win, cam); }
}

void Vine::submit(Renderer& renderer) {
	auto spro = vfx::SpriteRotator{};
	for (auto const& ball : m_treasure_balls) { ball->submit(renderer); }

	for (auto [i, link] : std::views::enumerate(m_chain.links)) {
		auto rotation_angle = (link.get_bob() - link.get_anchor()).normalized();
		auto idx = 0;
		if (m_angles > 1) { idx = spro.get_sprite_angle_index(); }
		auto const pos = util::round_to_even(link.get_bob() - p_animatable.get_f_dimensions());
		auto const& frame =
			sf::IntRect({static_cast<int>((static_cast<float>(i) / static_cast<float>(m_length)) * static_cast<float>(m_tapers) * idx) * p_animatable.get_dimensions().x, encodings.at(i).at(0) * p_animatable.get_dimensions().y},
						p_animatable.get_dimensions());
		sf::FloatRect dest{pos, sf::Vector2f{frame.size}};
		if (m_angles > 1) { spro.handle_rotation(p_animatable.get_sprite(), rotation_angle, m_angles); }
		renderer.submit(p_animatable.get_sprite().getTexture(), dest, frame, is_foreground() ? RenderLayer::foreground_entities : RenderLayer::background_entities);
	}
	for (auto const& plat : m_spawnable_platforms) { plat->submit(renderer); }
	for (auto const& mine : m_mines) { mine->submit(renderer); }
}

void Vine::add_platform(automa::ServiceProvider& svc, int link_index) {
	if (link_index > m_chain.links.size() || link_index < 0) { return; }
	auto& link = m_chain.links.at(link_index);
	m_spawnable_platforms.push_back(std::make_unique<entity::SpawnablePlatform>(svc, link.get_anchor(), link_index));
}

void Vine::add_mine(automa::ServiceProvider& svc, int link_index) {
	if (!m_map) { return; }
	if (link_index > m_chain.links.size() || link_index < 0) { return; }
	auto& link = m_chain.links.at(link_index);
	m_mines.push_back(std::make_unique<entity::Mine>(svc, *m_map.value(), entity::MineType::floating, link_index));
	m_mines.back()->set_position(m_chain.links.at(link_index).get_bob());
}

void Vine::remove_platform(int link_index) {
	std::erase_if(m_spawnable_platforms, [link_index](auto const& p) { return p->get_index() == link_index; });
}

void Vine::remove_mine(int link_index) {
	std::erase_if(m_mines, [link_index](auto const& m) { return m->get_index() == link_index; });
}

} // namespace fornani
