#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::audio {

class Effect {
  public:
	void update(float const time, float const x, float const y) { onUpdate(time, x, y); }
	void start() { onStart(); }
	void stop() { onStop(); }

	virtual ~Effect() = default;

  protected:
	explicit Effect(automa::ServiceProvider& svc);

  private:
	virtual void onUpdate(float time, float x, float y) = 0;
	virtual void onStart() = 0;
	virtual void onStop() = 0;

	virtual void onKey(sf::Keyboard::Key /*unused*/) {}
};

class Surround : public Effect {
  public:
	explicit Surround(automa::ServiceProvider& svc);
	void onUpdate(float /*time*/, [[maybe_unused]] float x, [[maybe_unused]] float y) override {}
	void onStart() override { m_music.play(); }
	void onStop() override { m_music.stop(); }

  private:
	sf::Vector2f m_position;
	sf::Music m_music;
};
} // namespace fornani::audio
