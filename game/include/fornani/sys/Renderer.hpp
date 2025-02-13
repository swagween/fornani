#pragma once

#include <SFML/Graphics.hpp>

namespace fornani::sys {
	class Renderer {

		void Draw(const sf::Drawable& drawable, const sf::RenderStates& states = sf::RenderStates::Default) {
			sf::RenderWindow* window = dynamic_cast<sf::RenderWindow*>(m_renderTarget);

			window->display();
		}

		sf::RenderTarget* m_renderTarget;
	};
} // namespace fornani::sys