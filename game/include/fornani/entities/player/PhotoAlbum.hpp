
#pragma once

#include <djson/json.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/entities/item/Postcard.hpp>
#include <vector>

namespace fornani {

class PhotoAlbum {
  public:
	void serialize(dj::Json& out) const;
	void unserialize(automa::ServiceProvider& svc, dj::Json const& in);

	void add_postcard(automa::ServiceProvider& svc, int index);
	void add_postcard(automa::ServiceProvider& svc, std::string_view tag);

  private:
	std::vector<Postcard> m_postcards{};
};

} // namespace fornani
