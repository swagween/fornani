
#include "fornani/entities/world/Inspectable.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/gui/Console.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::entity {

Inspectable::Inspectable(automa::ServiceProvider& svc, Vecu32 dim, Vecu32 pos, std::string_view key, int room_id, int alternates, int native, bool aoc)
	: scaled_dimensions(dim), scaled_position(pos), key(key), alternates(alternates), sprite(svc.assets.get_texture("inspectable_indicator")) {
	dimensions = static_cast<Vec>(dim * util::constants::u32_cell_size);
	position = static_cast<Vec>(pos * util::constants::u32_cell_size);
	bounding_box = shape::Shape(dimensions);
	bounding_box.set_position(position);
	animation.end();
	id = key.data() + std::to_string(room_id);
	native_id = native == 0 ? room_id : native;
	if (aoc) { attributes.set(InspectableAttributes::activate_on_contact); }
}

void Inspectable::update(automa::ServiceProvider& svc, player::Player& player, std::optional<std::unique_ptr<gui::Console>>& console, dj::Json& set) {
	position = static_cast<Vec>(scaled_position * util::constants::u32_cell_size);
	dimensions = static_cast<Vec>(scaled_dimensions * util::constants::u32_cell_size);
	bounding_box.set_position(position);
	flags.reset(InspectableFlags::activated);
	animation.update();

	// check for quest-based alternates
	auto quest_status = svc.quest.get_progression(fornani::QuestType::inspectable, native_id);
	if (quest_status > 0) { current_alt = quest_status; }

	if (bounding_box.overlaps(player.collider.hurtbox)) {
		if (!flags.test(InspectableFlags::hovered)) { flags.set(InspectableFlags::hovered_trigger); }
		flags.set(InspectableFlags::hovered);
		if (player.controller.inspecting() || attributes.test(InspectableAttributes::activate_on_contact)) { flags.set(InspectableFlags::activated); }
	} else {
		flags.reset(InspectableFlags::hovered);
	}
	if (flags.test(InspectableFlags::activated)) {
		for (auto& choice : set.array_view()) {
			if (choice["key"].as_string() == std::string{key}) { console = std::make_unique<gui::Console>(svc, choice, std::string{key + std::to_string(current_alt)}, gui::OutputType::instant); }
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

void Inspectable::render(automa::ServiceProvider& svc, sf::RenderWindow& win, Vec campos) {
	sf::RectangleShape box{};
	auto u = 0;
	auto v = animation.get_frame() * 32;
	sprite.setPosition(position + offset - campos);
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
		box.setSize(dimensions);
		win.draw(box);
	} else if (!attributes.test(InspectableAttributes::activate_on_contact)) {
		win.draw(sprite);
	}
}

} // namespace fornani::entity
