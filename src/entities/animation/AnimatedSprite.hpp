
#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>
#include <list>
#include <string_view>
#include <unordered_map>
#include "Animation.hpp"
#include "../../utils/BitFlags.hpp"
#include "../../utils/Cooldown.hpp"
#include "../../utils/Counter.hpp"

namespace automa {
struct ServiceProvider;
}

namespace anim {

class AnimatedSprite {
  public:
	AnimatedSprite() = default;
	AnimatedSprite(sf::Texture& texture, sf::Vector2<int> dimensions = {32, 32});
	void update(sf::Vector2<float> pos, int u = 0, int v = 0, bool horiz = false);
	void push_params(std::string_view label, Parameters in_params);
	void set_params(std::string_view label, bool force = false);
	void set_dimensions(sf::Vector2<int> dim);
	void set_origin(sf::Vector2<float> origin);
	void set_texture(sf::Texture& texture);
	void random_start(automa::ServiceProvider& svc);
	void end();
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	[[nodiscard]] auto get_frame() const -> int { return animation.get_frame(); }
	[[nodiscard]] auto just_started() const -> bool { return animation.just_started(); }
	[[nodiscard]] auto complete() -> bool { return animation.complete(); }
	Animation& get() { return animation; }

  private:
	sf::Sprite sprite{};
	sf::RectangleShape drawbox{};
	sf::Vector2<int> dimensions{};
	sf::Vector2<float> position{}; 
	Animation animation{};
	std::unordered_map<std::string_view, Parameters> params{};
};

} // namespace anim
