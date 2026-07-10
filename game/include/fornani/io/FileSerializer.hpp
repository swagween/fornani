
#pragma once

#include <fornani/core/Common.hpp>
#include <fornani/io/SaveHeader.hpp>
#include <filesystem>
#include <fstream>
#include <vector>

namespace fs = std::filesystem;

namespace fornani {

inline void write_u32(std::ostream& out, std::uint32_t v) {
	unsigned char bytes[4] = {static_cast<unsigned char>(v & 0xFF), static_cast<unsigned char>((v >> 8) & 0xFF), static_cast<unsigned char>((v >> 16) & 0xFF), static_cast<unsigned char>((v >> 24) & 0xFF)};

	out.write(reinterpret_cast<char const*>(bytes), 4);
}

inline std::uint32_t read_u32(std::istream& in) {
	unsigned char bytes[4];
	in.read(reinterpret_cast<char*>(bytes), 4);

	return (std::uint32_t(bytes[0])) | (std::uint32_t(bytes[1]) << 8) | (std::uint32_t(bytes[2]) << 16) | (std::uint32_t(bytes[3]) << 24);
}

template <std::size_t N>
void xor_transform(std::vector<char>& data, std::array<std::uint8_t, N> const& key) {
	for (std::size_t i = 0; i < data.size(); ++i) { data[i] ^= static_cast<char>(key[i % N]); }
}

inline bool save_json_blob(fs::path const& path, std::string const& json) {
	std::vector<char> data;
	data.reserve(json.size());
	data.assign(json.begin(), json.end());

	std::uint32_t hash = checksum(data);

	xor_transform(data, save_key_v);

	std::ofstream out(path, std::ios::binary);
	if (!out) return false;

	// header
	write_u32(out, magic_key_v);
	write_u32(out, save_version_v);
	write_u32(out, static_cast<std::uint32_t>(data.size()));

	// data
	out.write(data.data(), data.size());

	// checksum (original data)
	write_u32(out, hash);

	// verify write success
	if (!out) return false;

	return true;
}

inline static bool load_file(fs::path const& path, std::vector<char>& out_data) {
	std::ifstream in(path, std::ios::binary);
	if (!in) return false;

	SaveHeader header{};

	// read header safely
	header.magic = read_u32(in);
	header.version = read_u32(in);
	header.data_size = read_u32(in);

	if (!in) return false;

	// validate header
	if (header.magic != magic_key_v) return false;
	if (header.version != save_version_v) return false;

	// read data
	std::vector<char> data(header.data_size);
	in.read(data.data(), data.size());
	if (!in) return false;

	// read checksum
	std::uint32_t stored_hash = read_u32(in);
	if (!in) return false;

	// 🔑 decrypt FIRST
	xor_transform(data, save_key_v);

	// then verify
	std::uint32_t computed_hash = checksum(data);
	if (stored_hash != computed_hash) return false;

	out_data = std::move(data);
	return true;
}

inline bool is_binary_save(fs::path const& path) {
	std::ifstream in(path, std::ios::binary);
	if (!in) return false;

	std::uint32_t magic;
	in.read(reinterpret_cast<char*>(&magic), sizeof(magic));

	return magic == magic_key_v;
}

} // namespace fornani
