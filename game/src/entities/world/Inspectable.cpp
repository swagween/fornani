
#include "fornani/entities/world/Inspectable.hpp"
#include <fornani/gui/console/Console.hpp>
#include <fornani/systems/EventDispatcher.hpp>
#include <fornani/world/Map.hpp>
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::entity {

static bool b_destroy{};
static int b_id{};
static void destroy_me(int id) {
	b_destroy = true;
	b_id = id;
}

Inspectable::Inspectable(automa::ServiceProvider& svc, dj::Json const& in, int room, int index)
	: IWorldPositionable({in["position"][0].as<std::uint32_t>(), in["position"][1].as<std::uint32_t>()}, {in["dimensions"][0].as<std::uint32_t>(), in["dimensions"][1].as<std::uint32_t>()}),
	  sprite{svc.assets.get_texture("inspectable_indicator")}, m_index{index} {
	if (in["activate_on_contact"].as_bool()) { attributes.set(InspectableAttributes::activate_on_contact); }
	if (in["instant"].as_bool()) { attributes.set(InspectableAttributes::instant); }
	key = in["key"].as_string();
	m_label = key.data() + std::to_string(room);
	auto nat = in["id"].as<int>();
	native_id = nat == 0 ? room : nat;
	alternates = in["alternates"].as<int>();
	bounding_box = shape::Shape(get_world_dimensions());
	bounding_box.set_position(get_world_position());
	animation.end();
	svc.events.register_event(std::make_unique<Event<int>>("DestroyInspectable", &destroy_me));
}

void Inspectable::update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, [[maybe_unused]] player::Player& player) {
	auto const& set = svc.data.map_jsons.at(map.room_lookup).metadata["entities"]["inspectables"][m_index];
	bounding_box.set_position(get_world_position());
	flags.reset(InspectableFlags::activated);
	animation.update();
	if (m_indicator_cooldown.is_almost_complete()) { flags.reset(InspectableFlags::hovered); }
	if (b_destroy) {
		destroy_by_id(b_id);
		b_destroy = false;
	}
	if (flags.test(InspectableFlags::destroy) && !destroyed()) { svc.data.destroy_inspectable(native_id); }

	// check for quest-based alternates
	/*auto quest_status = svc.quest.get_progression(quest::QuestType::inspectable, native_id);
	if (quest_status > 0) { current_alt = quest_status; }*/

	if (bounding_box.overlaps(player.collider.hurtbox)) {
		if (!flags.test(InspectableFlags::hovered)) { flags.set(InspectableFlags::hovered_trigger); }
		flags.set(InspectableFlags::hovered);
		if (attributes.test(InspectableAttributes::activate_on_contact) && flags.test(InspectableFlags::can_engage)) {
			flags.set(InspectableFlags::activated);
			flags.reset(InspectableFlags::can_engage);
		}
		if (player.controller.inspecting()) { flags.set(InspectableFlags::activated); }
	} else {
		m_indicator_cooldown.update();
		if (!m_indicator_cooldown.running()) { m_indicator_cooldown.start(); }
		flags.set(InspectableFlags::can_engage);
	}
	if (flags.test(InspectableFlags::activated) && !player.is_busy()) {
		player.set_busy(true);
		auto output_type = attributes.test(InspectableAttributes::instant) ? gui::OutputType::instant : gui::OutputType::gradual;
		console = std::make_unique<gui::Console>(svc, set["series"][current_alt], output_type);
	}

	if (flags.test(InspectableFlags::hovered) && flags.consume(InspectableFlags::hovered_trigger) && animation.complete()) { animation.set_params(params); }
	if (console) {
		if (console.value()->get_key() == key) { flags.set(InspectableFlags::engaged); }
	}
	if (!console) {
		flags.reset(InspectableFlags::engaged);
		player.set_busy(false);
	}
}

void Inspectable::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f campos) {
	sf::RectangleShape box{};
	auto u = 0;
	auto v = animation.get_frame() * 32;
	sprite.setPosition(get_world_position() + offset - campos);
	sprite.setTextureRect(sf::IntRect{{u, v}, {32, 32}});

	if (svc.greyblock_mode()) {
		if (flags.test(InspectableFlags::activated)) {
			box.setFillColor(sf::Color{80, 180, 120, 100});
		} else {
			box.setFillColor(sf::Color{180, 120, 80, 100});
		}
		box.setOutlineColor(sf::Color::White);
		box.setOutlineThickness(-1);
		box.setPosition(bounding_box.get_position() - campos);
		box.setSize(get_world_dimensions());
		win.draw(box);
	} else if (!attributes.test(InspectableAttributes::activate_on_contact)) {
		win.draw(sprite);
	}
}

} // namespace fornani::entity
