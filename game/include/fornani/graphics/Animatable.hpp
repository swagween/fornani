
#pragma once

#include "fornani/entities/animation/Animation.hpp"
#include "fornani/graphics/Drawable.hpp"
#include "fornani/io/Logger.hpp"
#include "fornani/utils/Constants.hpp"

namespace fornani {
class Animatable : public Drawable {
  public:
	Animatable(automa::ServiceProvider& svc, std::string_view label, sf::Vector2i dimensions = constants::i_cell_vec);

	void set_parameters(anim::Parameters params) { animation.set_params(params); }
	void set_channel(int to) { m_channel = to; }
	void set_dimensions(sf::Vector2i const to) { m_dimensions = to; }
	void tick();

	/// <summary>
	/// @brief starts the animation at a random frame.
	/// </summary>
	void random_start();

	/// <summary>
	/// @brief starts the first frame of the animation at a random point. useful only for animations with one frame.
	/// </summary>
	void random_frame_start();

	[[nodiscard]] auto is_complete() -> bool { return animation.complete(); }
	[[nodiscard]] auto get_f_dimensions() const -> sf::Vector2f { return sf::Vector2f{m_dimensions}; }

	anim::Animation animation;

  private:
	sf::Vector2i m_dimensions;
	int m_channel{};
};
} // namespace fornani
