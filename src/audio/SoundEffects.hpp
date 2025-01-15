#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

namespace automa {
struct ServiceProvider;
}

namespace audio {

	class Effect {
  public:
	void update(float time, float x, float y) { onUpdate(time, x, y); }
	void start() { onStart(); }
	void stop() { onStop(); }

  protected:
	explicit Effect(automa::ServiceProvider& svc);

  private:
	virtual void onUpdate(float time, float x, float y) = 0;
	virtual void onStart() = 0;
	virtual void onStop() = 0;

	virtual void onKey(sf::Keyboard::Key) {}
	};
	class Surround : public Effect {
	  public:
		Surround(automa::ServiceProvider& svc);
		void onUpdate(float /*time*/, float x, float y) override {
		}
		void onStart() override { m_music.play(); }
		void onStop() override { m_music.stop(); }

	  private:
		sf::Vector2f m_position;
		sf::Music m_music;
	};
	}