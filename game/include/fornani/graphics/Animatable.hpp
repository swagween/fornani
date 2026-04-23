
#pragma once

#include <fornani/entities/animation/Animation.hpp>
#include <fornani/graphics/Drawable.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/utils/Constants.hpp>
#include <fornani/utils/TransparentStringHash.hpp>

namespace fornani {
class Animatable : public Drawable {
  public:
	Animatable(automa::ServiceProvider& svc, std::string_view label, sf::Vector2i dimensions = constants::i_cell_vec);

	void push_animation(std::string_view label, anim::Parameters params);
	void push_and_set_animation(std::string_view label, anim::Parameters params);
	void set_animation(std::string_view to);
	void set_parameters(anim::Parameters params) { animation.set_params(params); }
	void set_framerate(int to) { animation.params.framerate = to; }
	void set_channel(int to);
	void set_frame(int to);
	void set_dimensions(sf::Vector2i const to) { m_dimensions = to; }
	void tick();
	void check_for_switch();

	/// <summary>
	/// @brief starts the animation at a random frame.
	/// </summary>
	void random_start();

	/// <summary>
	/// @brief starts the first frame of the animation at a random point. useful only for animations with one frame.
	/// </summary>
	void random_frame_start();

	[[nodiscard]] auto is_animation(std::string_view check) -> bool { return check == m_current; }
	[[nodiscard]] auto is_complete() -> bool { return animation.complete(); }
	[[nodiscard]] auto get_dimensions() const -> sf::Vector2i { return m_dimensions; }
	[[nodiscard]] auto get_f_dimensions() const -> sf::Vector2f { return sf::Vector2f{m_dimensions}; }
	[[nodiscard]] auto get_animation_tag() const -> std::string_view { return m_current; }

	anim::Animation animation;

  protected:
	std::unordered_map<std::string, anim::Parameters, TransparentHash, TransparentEqual> p_animations{};

  private:
	void set_rect();

  private:
	std::string m_current{};
	std::string m_root_animation{};
	sf::Vector2i m_dimensions;
	int m_channel{};
};

} // namespace fornani
