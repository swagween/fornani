////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>

#include <cmath>
#include <cstdlib>
#include "SoundEffects.hpp"
#include "../service/ServiceProvider.hpp"

namespace {
constexpr auto pi = 3.14159265359f;
constexpr auto sqrt2 = 2.0f * 0.707106781186547524401f;

} // namespace

namespace audio {

class PitchVolume : public Effect {
  public:
	PitchVolume(automa::ServiceProvider& svc) : Effect(svc) {

		// Set the music to loop
		m_music.setLooping(true);

		// We don't care about attenuation in this effect
		m_music.setAttenuation(0.f);

		// Set initial pitch
		m_music.setPitch(m_pitch);

		// Set initial volume
		m_music.setVolume(m_volume);
	}

	void onUpdate(float /*time*/, float x, float y) override {
		m_pitch = std::clamp(2.f * x, 0.f, 2.f);
		m_volume = std::clamp(100.f * (1.f - y), 0.f, 100.f);

		m_music.setPitch(m_pitch);
		m_music.setVolume(m_volume);
	}

	void onStart() override {
		// We set the listener position back to the default
		// so that the music is right on top of the listener
		sf::Listener::setPosition({0.f, 0.f, 0.f});

		m_music.play();
	}

	void onStop() override { m_music.stop(); }

  private:
	float m_pitch{1.f};
	float m_volume{100.f};
	sf::Music m_music;
};

////////////////////////////////////////////////////////////
// Attenuation Effect
////////////////////////////////////////////////////////////
class Attenuation : public Effect {
  public:
	Attenuation(automa::ServiceProvider& svc) : Effect(svc) {
		// Set the music to loop
		m_music.setLooping(true);

		// Set attenuation factor
		m_music.setAttenuation(m_attenuation);

		// Set direction to face "downwards"
		m_music.setDirection({0.f, 1.f, 0.f});
	}

	void onUpdate(float /*time*/, float x, float y) override {
		m_music.setPosition({m_position.x, m_position.y, 0.f});
	}

	void onStart() override { m_music.play(); }

	void onStop() override { m_music.stop(); }

  private:
	sf::Vector2f m_position;
	sf::Music m_music;

	float m_attenuation{0.01f};
};

////////////////////////////////////////////////////////////
// Tone Generator
////////////////////////////////////////////////////////////
class Tone : public sf::SoundStream, public Effect {
  public:
	Tone(automa::ServiceProvider& svc) : Effect(svc) { sf::SoundStream::initialize(1, sampleRate, {sf::SoundChannel::Mono}); }

	void onUpdate(float /*time*/, float x, float y) override {
		m_amplitude = std::clamp(0.2f * (1.f - y), 0.f, 0.2f);
		m_frequency = std::clamp(500.f * x, 0.f, 500.f);
	}

	void onStart() override {
		// We set the listener position back to the default
		// so that the tone is right on top of the listener
		sf::Listener::setPosition({0.f, 0.f, 0.f});

		play();
	}

	void onStop() override { SoundStream::stop(); }

  private:
	bool onGetData(sf::SoundStream::Chunk& chunk) override {
		auto const period = 1.f / m_frequency;

		for (auto i = 0u; i < chunkSize; ++i) {
			auto value = 0.f;

			switch (m_type) {
			case Type::Sine: {
				value = m_amplitude * std::sin(2 * pi * m_frequency * m_time);
				break;
			}
			case Type::Square: {
				value = m_amplitude * (2 * (2 * std::floor(m_frequency * m_time) - std::floor(2 * m_frequency * m_time)) + 1);
				break;
			}
			case Type::Triangle: {
				value = 4 * m_amplitude / period * std::abs(std::fmod(((std::fmod((m_time - period / 4), period)) + period), period) - period / 2) - m_amplitude;
				break;
			}
			case Type::Sawtooth: {
				value = m_amplitude * 2 * (m_time / period - std::floor(0.5f + m_time / period));
				break;
			}
			}

			m_sampleBuffer[i] = static_cast<std::int16_t>(std::lround(value * std::numeric_limits<std::int16_t>::max()));
			m_time += timePerSample;
		}

		chunk.sampleCount = chunkSize;
		chunk.samples = m_sampleBuffer.data();

		return true;
	}

	void onSeek(sf::Time) override {
		// It doesn't make sense to seek in a tone generator
	}

	enum class Type { Sine, Square, Triangle, Sawtooth };

	static constexpr unsigned int sampleRate{44100};
	static constexpr std::size_t chunkSize{sampleRate / 100};
	static constexpr float timePerSample{1.f / float{sampleRate}};

	std::vector<std::int16_t> m_sampleBuffer = std::vector<std::int16_t>(chunkSize, 0);
	Type m_type{Type::Triangle};
	float m_amplitude{0.05f};
	float m_frequency{220};
	float m_time{};
};

////////////////////////////////////////////////////////////
// Dopper Shift Effect
////////////////////////////////////////////////////////////
class Doppler : public sf::SoundStream, public Effect {
  public:
	Doppler(automa::ServiceProvider& svc) : Effect(svc) {

		// Set attenuation to a nice value
		setAttenuation(0.05f);

		sf::SoundStream::initialize(1, sampleRate, {sf::SoundChannel::Mono});
	}

	void onUpdate(float time, float x, float y) override {
		m_velocity = std::clamp(150.f * (1.f - y), 0.f, 150.f);
		m_factor = std::clamp(x, 0.f, 1.f);

		setPosition({m_position.x, m_position.y, 0.f});
	}

	void onStart() override { play(); }

	void onStop() override { SoundStream::stop(); }

  private:
	bool onGetData(sf::SoundStream::Chunk& chunk) override {
		auto const period = 1.f / m_frequency;

		for (auto i = 0u; i < chunkSize; ++i) {
			auto const value = m_amplitude * 2 * (m_time / period - std::floor(0.5f + m_time / period));

			m_sampleBuffer[i] = static_cast<std::int16_t>(std::lround(value * std::numeric_limits<std::int16_t>::max()));
			m_time += timePerSample;
		}

		chunk.sampleCount = chunkSize;
		chunk.samples = m_sampleBuffer.data();

		return true;
	}

	void onSeek(sf::Time) override {
		// It doesn't make sense to seek in a tone generator
	}

	static constexpr unsigned int sampleRate{44100};
	static constexpr std::size_t chunkSize{sampleRate / 100};
	static constexpr float timePerSample{1.f / float{sampleRate}};

	std::vector<std::int16_t> m_sampleBuffer = std::vector<std::int16_t>(chunkSize, 0);
	float m_amplitude{0.05f};
	float m_frequency{220};
	float m_time{};

	float m_velocity{0.f};
	float m_factor{1.f};
	sf::Vector2f m_position;
};

////////////////////////////////////////////////////////////
// Processing base class
////////////////////////////////////////////////////////////
class Processing : public Effect {
  public:
	void onUpdate([[maybe_unused]] float time, float x, float y) override {
		m_music.setPosition({m_position.x, m_position.y, 0.f});
	}

	void onStart() override { m_music.play(); }

	void onStop() override { m_music.stop(); }

  protected:
	explicit Processing(automa::ServiceProvider& svc) : Effect(svc) {
		m_music.setLooping(true);
		m_music.setAttenuation(0.0f);
	}

	sf::Music& getMusic() { return m_music; }

	std::shared_ptr<bool> const& getEnabled() const { return m_enabled; }

  private:
	void onKey(sf::Keyboard::Key key) override {
		if (key == sf::Keyboard::Key::Space) *m_enabled = !*m_enabled;
	}

	sf::Vector2f m_position;
	sf::Music m_music;
	std::shared_ptr<bool> m_enabled{std::make_shared<bool>(true)};
};

////////////////////////////////////////////////////////////
// Biquad Filter (https://github.com/dimtass/DSP-Cpp-filters)
////////////////////////////////////////////////////////////
class BiquadFilter : public Processing {
  protected:
	struct Coefficients {
		float a0{};
		float a1{};
		float a2{};
		float b1{};
		float b2{};
		float c0{};
		float d0{};
	};

	using Processing::Processing;

	void setCoefficients(Coefficients const& coefficients) {
		auto& music = getMusic();

		struct State {
			float xnz1{};
			float xnz2{};
			float ynz1{};
			float ynz2{};
		};

		// We use a mutable lambda to tie the lifetime of the state and coefficients to the lambda itself
		// This is necessary since the Echo object will be destroyed before the Music object
		// While the Music object exists, it is possible that the audio engine will try to call
		// this lambda hence we need to always have usable coefficients and state until the Music and the
		// associated lambda are destroyed
		music.setEffectProcessor(
			[coefficients, enabled = getEnabled(), state = std::vector<State>()](float const* inputFrames, unsigned int& inputFrameCount, float* outputFrames, unsigned int& outputFrameCount, unsigned int frameChannelCount) mutable {
				// IMPORTANT: The channel count of the audio engine currently sourcing data from this sound
				// will always be provided in frameChannelCount, this can be different from the channel count
				// of the audio source so make sure to size your buffers according to the engine and not the source
				// Ensure we have as many state objects as the audio engine has channels
				if (state.size() < frameChannelCount) state.resize(frameChannelCount - state.size());

				for (auto frame = 0u; frame < outputFrameCount; ++frame) {
					for (auto channel = 0u; channel < frameChannelCount; ++channel) {
						auto& channelState = state[channel];

						const auto xn = inputFrames ? inputFrames[channel] : 0.f; // Read silence if no input data available
						const auto yn = coefficients.a0 * xn + coefficients.a1 * channelState.xnz1 + coefficients.a2 * channelState.xnz2 - coefficients.b1 * channelState.ynz1 - coefficients.b2 * channelState.ynz2;

						channelState.xnz2 = channelState.xnz1;
						channelState.xnz1 = xn;
						channelState.ynz2 = channelState.ynz1;
						channelState.ynz1 = yn;

						outputFrames[channel] = *enabled ? yn : xn;
					}

					inputFrames += (inputFrames ? frameChannelCount : 0u);
					outputFrames += frameChannelCount;
				}

				// We processed data 1:1
				inputFrameCount = outputFrameCount;
			});
	}
};

////////////////////////////////////////////////////////////
// High-pass Filter (https://github.com/dimtass/DSP-Cpp-filters)
////////////////////////////////////////////////////////////
struct HighPassFilter : BiquadFilter {
	HighPassFilter(automa::ServiceProvider& svc) : BiquadFilter(svc) {
		static constexpr auto cutoffFrequency = 2000.f;

		auto const c = std::tan(pi * cutoffFrequency / static_cast<float>(getMusic().getSampleRate()));

		Coefficients coefficients;

		coefficients.a0 = 1.f / (1.f + sqrt2 * c + std::pow(c, 2.f));
		coefficients.a1 = -2.f * coefficients.a0;
		coefficients.a2 = coefficients.a0;
		coefficients.b1 = 2.f * coefficients.a0 * (std::pow(c, 2.f) - 1.f);
		coefficients.b2 = coefficients.a0 * (1.f - sqrt2 * c + std::pow(c, 2.f));

		setCoefficients(coefficients);
	}
};

////////////////////////////////////////////////////////////
// Low-pass Filter (https://github.com/dimtass/DSP-Cpp-filters)
////////////////////////////////////////////////////////////
struct LowPassFilter : BiquadFilter {
	LowPassFilter(automa::ServiceProvider& svc) : BiquadFilter(svc) {
		static constexpr auto cutoffFrequency = 500.f;

		auto const c = 1.f / std::tan(pi * cutoffFrequency / static_cast<float>(getMusic().getSampleRate()));

		Coefficients coefficients;

		coefficients.a0 = 1.f / (1.f + sqrt2 * c + std::pow(c, 2.f));
		coefficients.a1 = 2.f * coefficients.a0;
		coefficients.a2 = coefficients.a0;
		coefficients.b1 = 2.f * coefficients.a0 * (1.f - std::pow(c, 2.f));
		coefficients.b2 = coefficients.a0 * (1.f - sqrt2 * c + std::pow(c, 2.f));

		setCoefficients(coefficients);
	}
};

////////////////////////////////////////////////////////////
// Echo (miniaudio implementation)
////////////////////////////////////////////////////////////
struct Echo : Processing {
	Echo(automa::ServiceProvider& svc) : Processing(svc) {
		auto& music = getMusic();

		static constexpr auto delay = 0.2f;
		static constexpr auto decay = 0.75f;
		static constexpr auto wet = 0.8f;
		static constexpr auto dry = 1.f;

		auto const sampleRate = music.getSampleRate();
		auto const delayInFrames = static_cast<unsigned int>(static_cast<float>(sampleRate) * delay);

		// We use a mutable lambda to tie the lifetime of the state to the lambda itself
		// This is necessary since the Echo object will be destroyed before the Music object
		// While the Music object exists, it is possible that the audio engine will try to call
		// this lambda hence we need to always have a usable state until the Music and the
		// associated lambda are destroyed
		music.setEffectProcessor([delayInFrames, enabled = getEnabled(), buffer = std::vector<float>(), cursor = 0u](float const* inputFrames, unsigned int& inputFrameCount, float* outputFrames, unsigned int& outputFrameCount,
																													 unsigned int frameChannelCount) mutable {
			// IMPORTANT: The channel count of the audio engine currently sourcing data from this sound
			// will always be provided in frameChannelCount, this can be different from the channel count
			// of the audio source so make sure to size your buffers according to the engine and not the source
			// Ensure we have enough space to store the delayed frames for all of the audio engine's channels
			if (buffer.size() < delayInFrames * frameChannelCount) buffer.resize(delayInFrames * frameChannelCount - buffer.size(), 0.f);

			for (auto frame = 0u; frame < outputFrameCount; ++frame) {
				for (auto channel = 0u; channel < frameChannelCount; ++channel) {
					const auto input = inputFrames ? inputFrames[channel] : 0.f; // Read silence if no input data available
					const auto bufferIndex = (cursor * frameChannelCount) + channel;
					buffer[bufferIndex] = (buffer[bufferIndex] * decay) + (input * dry);
					outputFrames[channel] = *enabled ? buffer[bufferIndex] * wet : input;
				}

				cursor = (cursor + 1) % delayInFrames;

				inputFrames += (inputFrames ? frameChannelCount : 0u);
				outputFrames += frameChannelCount;
			}

			// We processed data 1:1
			inputFrameCount = outputFrameCount;
		});
	}
};

////////////////////////////////////////////////////////////
// Reverb (https://github.com/sellicott/DSP-FFMpeg-Reverb)
////////////////////////////////////////////////////////////
class Reverb : public Processing {
  public:
	Reverb(automa::ServiceProvider& svc) : Processing(svc) {
		auto& music = getMusic();

		static constexpr auto sustain = 0.7f; // [0.f; 1.f]

		// We use a mutable lambda to tie the lifetime of the state to the lambda itself
		// This is necessary since the Echo object will be destroyed before the Music object
		// While the Music object exists, it is possible that the audio engine will try to call
		// this lambda hence we need to always have a usable state until the Music and the
		// associated lambda are destroyed
		music.setEffectProcessor([sampleRate = music.getSampleRate(), filters = std::vector<ReverbFilter<float>>(), enabled = getEnabled()](float const* inputFrames, unsigned int& inputFrameCount, float* outputFrames,
																																			unsigned int& outputFrameCount, unsigned int frameChannelCount) mutable {
			// IMPORTANT: The channel count of the audio engine currently sourcing data from this sound
			// will always be provided in frameChannelCount, this can be different from the channel count
			// of the audio source so make sure to size your buffers according to the engine and not the source
			// Ensure we have as many filter objects as the audio engine has channels
			while (filters.size() < frameChannelCount) filters.emplace_back(sampleRate, sustain);

			for (auto frame = 0u; frame < outputFrameCount; ++frame) {
				for (auto channel = 0u; channel < frameChannelCount; ++channel) {
					const auto input = inputFrames ? inputFrames[channel] : 0.f; // Read silence if no input data available
					outputFrames[channel] = *enabled ? filters[channel](input) : input;
				}

				inputFrames += (inputFrames ? frameChannelCount : 0u);
				outputFrames += frameChannelCount;
			}

			// We processed data 1:1
			inputFrameCount = outputFrameCount;
		});
	}

  private:
	template <typename T>
	class AllPassFilter {
	  public:
		AllPassFilter(std::size_t delay, float theGain) : m_buffer(delay, {}), m_gain(theGain) {}

		T operator()(T input) {
			auto const output = m_buffer[m_cursor];
			input = static_cast<T>(input + m_gain * output);
			m_buffer[m_cursor] = input;
			m_cursor = (m_cursor + 1) % m_buffer.size();
			return static_cast<T>(-m_gain * input + output);
		}

	  private:
		std::vector<T> m_buffer;
		std::size_t m_cursor{};
		float const m_gain{};
	};

	template <typename T>
	class FIRFilter {
	  public:
		explicit FIRFilter(std::vector<float> taps) : m_taps(std::move(taps)) {}

		T operator()(T input) {
			m_buffer[m_cursor] = input;
			m_cursor = (m_cursor + 1) % m_buffer.size();

			T output{};

			for (auto i = 0u; i < m_taps.size(); ++i) output += static_cast<T>(m_taps[i] * m_buffer[(m_cursor + i) % m_buffer.size()]);

			return output;
		}

	  private:
		std::vector<float> const m_taps;
		std::vector<T> m_buffer = std::vector<T>(m_taps.size(), {});
		std::size_t m_cursor{};
	};

	template <typename T>
	class ReverbFilter {
	  public:
		ReverbFilter(unsigned int sampleRate, float feedbackGain)
			: m_allPass{{{sampleRate / 10, 0.6f}, {sampleRate / 30, -0.6f}, {sampleRate / 90, 0.6f}, {sampleRate / 270, -0.6f}}},
			  m_fir({0.003369f, 0.002810f, 0.001758f,  0.000340f,  -0.001255f, -0.002793f, -0.004014f, -0.004659f, -0.004516f, -0.003464f, -0.001514f, 0.001148f,  0.004157f,  0.006986f,  0.009003f, 0.009571f,
					 0.008173f, 0.004560f, -0.001120f, -0.008222f, -0.015581f, -0.021579f, -0.024323f, -0.021933f, -0.012904f, 0.003500f,  0.026890f,  0.055537f,  0.086377f,  0.115331f,  0.137960f, 0.150407f,
					 0.150407f, 0.137960f, 0.115331f,  0.086377f,  0.055537f,  0.026890f,  0.003500f,  -0.012904f, -0.021933f, -0.024323f, -0.021579f, -0.015581f, -0.008222f, -0.001120f, 0.004560f, 0.008173f,
					 0.009571f, 0.009003f, 0.006986f,  0.004157f,  0.001148f,  -0.001514f, -0.003464f, -0.004516f, -0.004659f, -0.004014f, -0.002793f, -0.001255f, 0.000340f,  0.001758f,  0.002810f, 0.003369f}),
			  m_buffer(sampleRate / 5, {}), // sample rate / 5 = 200ms buffer size
			  m_feedbackGain(feedbackGain) {}

		T operator()(T input) {
			auto output = static_cast<T>(0.7f * input + m_feedbackGain * m_buffer[m_cursor]);

			for (auto& f : m_allPass) output = f(output);

			output = m_fir(output);

			m_buffer[m_cursor] = output;
			m_cursor = (m_cursor + 1) % m_buffer.size();

			output += 0.5f * m_buffer[(m_cursor + 1 * m_interval - 1) % m_buffer.size()];
			output += 0.25f * m_buffer[(m_cursor + 2 * m_interval - 1) % m_buffer.size()];
			output += 0.125f * m_buffer[(m_cursor + 3 * m_interval - 1) % m_buffer.size()];

			return 0.6f * output + input;
		}

	  private:
		std::array<AllPassFilter<T>, 4> m_allPass;
		FIRFilter<T> m_fir;
		std::vector<T> m_buffer;
		std::size_t m_cursor{};
		std::size_t const m_interval{m_buffer.size() / 3};
		float const m_feedbackGain{};
	};
};

Surround::Surround(automa::ServiceProvider& svc) : Effect(svc) {

	// Set the music to loop
	m_music.setLooping(true);

	// Set attenuation to a nice value
	m_music.setAttenuation(0.04f);
}

Effect::Effect(automa::ServiceProvider& svc) {}

} // namespace audio