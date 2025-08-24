
#include <fornani/entities/player/Player.hpp>
#include <fornani/entity/Vine.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Math.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

constexpr auto segment_size_v = sf::Vector2i{64, 64};

Vine::Vine(automa::ServiceProvider& svc, int length, int size, bool foreground, bool reversed, std::vector<int> const platform_indeces)
	: Entity(svc, "vines", 0), m_length(length), m_chain(svc, {0.995f, 0.08f, static_cast<float>(size) * 0.5f, 14.f}, get_world_position(), length, reversed), m_services(&svc), m_spacing{0.f, 128.f} {
	for (auto const& i : platform_indeces) {
		if (i == -1) { continue; }
		add_platform(svc, i);
	}
	init();
}

Vine::Vine(automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "vines", segment_size_v), m_services(&svc), m_chain(svc, {0.995f, 0.06f, 16.f, 14.f}, get_world_position(), in["length"].as<int>()), m_spacing{0.f, 128.f} {
	unserialize(in);
	init();
}

void Vine::init() {
	Animatable::center();
	auto index = util::Circuit(4);
	auto last_index = random::random_range(0, 3);
	auto ctr{0};
	for (auto& link : m_chain.links) {
		index.set(random::random_range(0, index.get_order()));
		if (index.get() == last_index) { index.modulate(1); }
		auto const sign = random::percent_chance(50) ? -1 : 1;
		encodings.push_back({index.get(), sign});
		last_index = index.get();
		// optionally add treasure container to vine segment
		if (random::percent_chance(8)) {
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
			if (!m_treasure_balls) { m_treasure_balls = std::vector<std::unique_ptr<entity::TreasureContainer>>{}; }
			m_treasure_balls.value().push_back(std::make_unique<entity::TreasureContainer>(*m_services, rarity, link.get_anchor(), ctr));
		}
		++ctr;
	}
	repeatable = false;
}

std::unique_ptr<Entity> Vine::clone() const { return std::make_unique<Vine>(*this); }

void Vine::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["foreground"] = is_foreground();
	out["length"] = m_length;
	if (m_spawnable_platforms) {
		for (auto const& plat : m_spawnable_platforms.value()) { out["platform"]["link_indeces"].push_back(plat->get_index()); }
	}
}

void Vine::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	in["foreground"].as_bool() ? m_flags.set(VineFlags::foreground) : m_flags.reset(VineFlags::foreground);
	m_length = in["length"].as<int>();
	if (in["platform"]) {
		for (auto& link : in["platform"]["link_indeces"].as_array()) { add_platform(*m_services, link.as<int>()); }
	}
}

void Vine::expose() { Entity::expose(); }

void Vine::update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, [[maybe_unused]] player::Player& player) {
	Entity::update(svc, map, console, player);
	m_chain.set_position(get_world_position());
	m_chain.update(svc, map, player);
	if (m_treasure_balls) {
		for (auto const& ball : m_treasure_balls.value()) { ball->update(svc, m_chain.links.at(ball->get_index()).get_bob()); }
		std::erase_if(m_treasure_balls.value(), [](auto const& b) { return b->destroyed(); });
	}
	if (m_spawnable_platforms) {
		for (auto const& plat : m_spawnable_platforms.value()) { plat->update(svc, player, m_chain.links.at(plat->get_index()).get_bob()); }
	}
}

void Vine::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) const {
	if (m_treasure_balls) {
		for (auto& ball : m_treasure_balls.value()) { ball->on_hit(svc, map, proj); }
	}
	if (m_spawnable_platforms) {
		for (auto& plat : m_spawnable_platforms.value()) { plat->on_hit(svc, map, proj); }
	}
}

void Vine::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{60, 255, 120, 180}) : drawbox.setFillColor(sf::Color{60, 255, 120, 80});
	Entity::render(win, cam, size);
	if (m_editor) { return; }
	if (m_services->greyblock_mode()) { m_chain.render(*m_services, win, cam); }
	if (m_treasure_balls) {
		for (auto const& ball : m_treasure_balls.value()) { ball->render(*m_services, win, cam); }
	}
	int ctr{0};
	auto current = 0.f;
	auto total = static_cast<float>(m_chain.links.size());
	for (auto& link : m_chain.links) {
		Animatable::set_texture_rect(sf::IntRect({static_cast<int>((current / static_cast<float>(m_length)) * 3.f) * segment_size_v.x, encodings.at(ctr).at(0) * segment_size_v.y}, segment_size_v));
		Animatable::set_scale(sf::Vector2f{static_cast<float>(encodings.at(ctr).at(1)), 1.f} * constants::f_scale_factor);
		Animatable::set_position(util::round_to_even(link.get_bob()) - cam);
		win.draw(*this);
		++ctr;
		++current;
	}
	if (m_spawnable_platforms) {
		for (auto const& plat : m_spawnable_platforms.value()) { plat->render(*m_services, win, cam); }
	}
}

void Vine::add_platform(automa::ServiceProvider& svc, int link_index) {
	m_treasure_balls = {}; // don't want them to get in the way
	if (link_index > m_chain.links.size() || link_index < 0) { return; }
	auto& link = m_chain.links.at(link_index);
	if (!m_spawnable_platforms) { m_spawnable_platforms = std::vector<std::unique_ptr<entity::SpawnablePlatform>>{}; }
	m_spawnable_platforms.value().push_back(std::make_unique<entity::SpawnablePlatform>(svc, link.get_anchor(), link_index));
}

} // namespace fornani
