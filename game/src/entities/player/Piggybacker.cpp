
#include <fornani/entities/player/Piggybacker.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::player {

Piggybacker::Piggybacker(automa::ServiceProvider& svc, std::string_view label, sf::Vector2f position)
	: Animatable(svc, "npc_" + std::string{label}, {svc.data.npc[label]["sprite_dimensions"][0].as<int>(), svc.data.npc[label]["sprite_dimensions"][1].as<int>()}), m_id{svc.data.npc[label]["id"].as<int>()} {
	center();
	NANI_LOG_DEBUG(m_logger, "Created a Piggybacker with Label {}", label);
	auto const& in_anim = svc.data.npc[label]["animation"];
	for (auto const& anim : in_anim.as_array()) {
		if (anim["label"].as_string() == "piggyback") { set_parameters({anim["parameters"][0].as<int>(), anim["parameters"][1].as<int>(), anim["parameters"][2].as<int>(), anim["parameters"][3].as<int>(), anim["parameters"][4].as_bool()}); }
	}
	m_steering.physics.position = position;
}

void Piggybacker::update(automa::ServiceProvider& svc, Player& player) {
	m_steering.physics.position = player.get_piggyback_socket();
	/*m_steering.seek(player.get_piggyback_socket(), 0.02f);*/
	set_scale(player.get_actual_direction().right() ? constants::f_scale_vec : constants::f_inverse_scale_vec);
	tick();
}

void Piggybacker::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Animatable::set_position(m_steering.physics.position - cam);
	win.draw(*this);
}

} // namespace fornani::player
