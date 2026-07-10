
#pragma once

#include <fornani/core/Common.hpp>
#include <fornani/io/FileSerializer.hpp>
#include <string>

namespace fornani::codec {

inline bool encode(std::string const& json, std::ostream& out) {
	std::vector<char> data(json.begin(), json.end());

	std::uint32_t hash = checksum(data);

	xor_transform(data, save_key_v);

	write_u32(out, magic_key_v);
	write_u32(out, save_version_v);
	write_u32(out, static_cast<std::uint32_t>(data.size()));

	out.write(data.data(), data.size());
	write_u32(out, hash);

	return static_cast<bool>(out);
}

inline bool decode(std::istream& in, std::string& out_json) {
	std::uint32_t magic = read_u32(in);
	std::uint32_t version = read_u32(in);
	std::uint32_t size = read_u32(in);

	if (!in) return false;
	if (magic != magic_key_v) return false;
	if (version != save_version_v) return false;

	std::vector<char> data(size);
	in.read(data.data(), size);
	if (!in) return false;

	std::uint32_t stored_hash = read_u32(in);
	if (!in) return false;

	xor_transform(data, save_key_v);

	if (checksum(data) != stored_hash) return false;

	out_json.assign(data.begin(), data.end());
	return true;
}

} // namespace fornani::codec
