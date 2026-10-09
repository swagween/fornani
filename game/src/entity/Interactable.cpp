
#include <fornani/entity/Interactable.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani {

Interactable::Interactable(automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "interactables") {
	unserialize(in);
	auto const& in_data = svc.data.interactables[m_tag];
	for (auto const& a : in_data["animation"].as_array()) { p_animatable.push_and_set_animation(a["label"].as_string(), anim::Parameters::from_json(a["params"])); }
	m_offset = sf::Vector2f{in_data["offset"][0].as<float>(), in_data["offset"][1].as<float>()};
	p_animatable.set_texture(svc.assets.get_texture("interactable_" + m_tag));
	p_animatable.set_dimensions(sf::Vector2i{in_data["dimensions"][0].as<int>(), in_data["dimensions"][1].as<int>()});
	p_animatable.tick();
	p_animatable.center();
}

Interactable::Interactable(automa::ServiceProvider& svc, std::string_view tag, int channel) : Entity(svc, "interactables", 0), m_tag{tag}, m_channel{channel} {}

std::unique_ptr<Entity> Interactable::clone() const { return std::make_unique<Interactable>(*this); }

void Interactable::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["tag"] = m_tag;
}

void Interactable::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	m_tag = in["tag"].as_string();
}

void Interactable::expose() { Entity::expose(); }

void Interactable::update(automa::ServiceProvider& svc, world::Map& map, SceneContext& context, player::Player& player) { Entity::update(svc, map, context, player); }

void Interactable::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{60, 60, 120, 180}) : drawbox.setFillColor(sf::Color{60, 60, 120, 80});
	Entity::render(win, cam, size);
	if (spawn_denied()) { return; }
	if (m_editor) { return; }
	p_animatable.set_position(get_global_center() - cam + m_offset);
	win.draw(p_animatable);
	if (debug::is_debug()) {}
}

} // namespace fornani
