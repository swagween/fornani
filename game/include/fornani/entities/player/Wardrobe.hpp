
#pragma once
#include <SFML/Graphics.hpp>
#include <string_view>
#include <unordered_map>
#include "fornani/utils/BitFlags.hpp"

#include "fornani/io/Logger.hpp"

namespace fornani::flfx {
class TextureUpdater;
}

namespace fornani::player {

enum class ApparelType : std::uint8_t { hairstyle, headgear, shirt, pants, END };
enum class ApparelPants : std::uint8_t { standard, red_jeans, chalcedony_skirt, punk_pants };
enum class ApparelShirt : std::uint8_t { standard, skycorps_tee, chalcedony_shirt, punk_shirt };
enum class ApparelHeadgear : std::uint8_t { standard, head_lamp, gas_mask, sporty_cap };
enum class ApparelHairstyle : std::uint8_t { standard, bob_cut, ponytail, punk_hair };

class Apparel {
  public:
	virtual int get_variant() const { return 0; };
	virtual void set(int to) {}
};

class Pants : public Apparel {
  public:
	int get_variant() const override { return static_cast<int>(m_variant); }
	void set(int to) override { m_variant = static_cast<ApparelPants>(to); }

  private:
	ApparelPants m_variant{};
};

class Shirt : public Apparel {
  public:
	int get_variant() const override { return static_cast<int>(m_variant); }
	void set(int to) override { m_variant = static_cast<ApparelShirt>(to); }

  private:
	ApparelShirt m_variant{};
};

class Headgear : public Apparel {
  public:
	int get_variant() const override { return static_cast<int>(m_variant); }
	void set(int to) override { m_variant = static_cast<ApparelHeadgear>(to); }

  private:
	ApparelHeadgear m_variant{};
};

class Hairstyle : public Apparel {
  public:
	int get_variant() const override { return static_cast<int>(m_variant); }
	void set(int to) override { m_variant = static_cast<ApparelHairstyle>(to); }

  private:
	ApparelHairstyle m_variant{};
};

class Wardrobe {
  public:
	Wardrobe();
	void set_palette(sf::Texture& tex);
	void change_outfit(std::vector<std::pair<sf::Vector2<unsigned int>, sf::Color>> replacement);
	void update(flfx::TextureUpdater& updater);
	void equip(ApparelType type, int variant);
	void unequip(ApparelType type);
	int get_variant(ApparelType type);
	std::array<int, static_cast<int>(ApparelType::END)> get();

  private:
	std::unordered_map<ApparelType, std::unique_ptr<Apparel>> m_outfit{};
	sf::Texture m_palette{};

	io::Logger m_logger{"entities"};
};

} // namespace fornani::player
