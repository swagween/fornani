
#pragma once

#include <fornani/core/Fwd.hpp>
#include <fornani/graphics/Drawable.hpp>
#include <fornani/shader/LightShader.hpp>
#include <fornani/utils/RectPath.hpp>

namespace fornani::gui {

struct VendorConstituent final : public Drawable {
	explicit VendorConstituent(automa::ServiceProvider& svc, std::string_view label, sf::IntRect lookup, int speed = 128, util::InterpolationType type = util::InterpolationType::quadratic);
	util::RectPath path;
	void update();
	void render(sf::RenderWindow& win, LightShader& shader, Palette& palette);
};

} // namespace fornani::gui
