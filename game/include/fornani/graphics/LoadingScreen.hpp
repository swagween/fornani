
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/gui/StatusBar.hpp>
#include <fornani/io/Logger.hpp>

namespace fornani {

class LoadingScreen {
  public:
	LoadingScreen(automa::ServiceProvider& svc);
	void render(sf::RenderWindow& win, float progress);
	void stall(sf::RenderWindow& win);

  private:
	gui::StatusBar m_bar;
	automa::ServiceProvider* m_services;
	sf::Text m_readout;

	io::Logger m_logger{"LoadingScreen"};
};

} // namespace fornani
