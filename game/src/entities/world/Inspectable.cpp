
#include "fornani/entities/world/Inspectable.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/gui/Console.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::entity {

Inspectable::Inspectable(automa::ServiceProvider& svc, dj::Json const& in, int room)
	: IWorldPositionable({in["position"][0].as<std::uint32_t>(), in["position"][1].as<std::uint32_t>()}, {in["dimensions"][0].as<std::uint32_t>(), in["dimensions"][1].as<std::uint32_t>()}),
	  sprite{svc.assets.get_texture("inspectable_indicator")} {
	if (in["activate_on_contact"].as_bool()) { attributes.set(InspectableAttributes::activate_on_contact); }
	if (in["instant"].as_bool()) { attributes.set(InspectableAttributes::instant); }
	key = in["key"].as_string();
	id = key.data() + std::to_string(room);
	auto nat = in["native_id"].as<int>();
	native_id = nat == 0 ? room : nat;
	alternates = in["alternates"].as<int>();
	bounding_box = shape::Shape(get_world_dimensions());
	bounding_box.set_position(get_world_position());
	animation.end();
}

Inspectable::Inspectable(automa::ServiceProvider& svc, Vecu32 dim, Vecu32 pos, std::string_view key, int room_id, int alternates, int native, bool aoc, bool instant)
	: IWorldPositionable(pos, dim), key(key), alternates(alternates), sprite(svc.assets.get_texture("inspectable_indicator")) {
	bounding_box = shape::Shape(get_world_dimensions());
	bounding_box.set_position(get_world_position());
	animation.end();
	id = key.data() + std::to_string(room_id);
	native_id = native == 0 ? room_id : native;
	if (aoc) { attributes.set(InspectableAttributes::activate_on_contact); }
	if (instant) { attributes.set(InspectableAttributes::instant); }
}

void Inspectable::update(automa::ServiceProvider& svc, player::Player& player, std::optional<std::unique_ptr<gui::Console>>& console, dj::Json const& set) {
	bounding_box.set_position(get_world_position());
	flags.reset(InspectableFlags::activated);
	animation.update();
	m_indicator_cooldown.update();
	if (m_indicator_cooldown.is_almost_complete()) { flags.reset(InspectableFlags::hovered); }

	// check for quest-based alternates
	auto quest_status = svc.quest.get_progression(fornani::QuestType::inspectable, native_id);
	if (quest_status > 0) { current_alt = quest_status; }

	if (bounding_box.overlaps(player.collider.hurtbox)) {
		if (!flags.test(InspectableFlags::hovered)) { flags.set(InspectableFlags::hovered_trigger); }
		flags.set(InspectableFlags::hovered);
		if (player.controller.inspecting() || attributes.test(InspectableAttributes::activate_on_contact)) { flags.set(InspectableFlags::activated); }
	} else {
		if (!m_indicator_cooldown.running()) { m_indicator_cooldown.start(); }
	}
	if (flags.test(InspectableFlags::activated)) {
		for (auto const& choice : set.as_array()) {
			auto output_type = attributes.test(InspectableAttributes::instant) ? gui::OutputType::instant : gui::OutputType::gradual;
			if (choice["key"].as_string() == std::string{key}) { console = std::make_unique<gui::Console>(svc, choice, std::string{key + std::to_string(current_alt)}, output_type); }
		}
	}
	if (flags.test(InspectableFlags::hovered) && flags.consume(InspectableFlags::hovered_trigger) && animation.complete()) { animation.set_params(params); }
	if (console) {
		if (console.value()->get_key() == key) { flags.set(InspectableFlags::engaged); }
	}
	if (flags.test(InspectableFlags::engaged)) {
		// if (player.transponder.shipments.quest.get_residue() == 9) {
		//  TODO: properly handle inspectable codes from console
		if (0 == 9) {
			flags.set(InspectableFlags::destroy);
			svc.data.destroy_inspectable(id);
		}
	}
	if (!console) { flags.reset(InspectableFlags::engaged); }
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
