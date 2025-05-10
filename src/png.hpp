#ifndef PNG_HPP
#define PNG_HPP

#ifdef _DEBUG
#include <iostream>
#endif

#include <string>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include "pixel.hpp"

void make_crc_table();
uint64_t update_crc(uint64_t crc, char *buffer, size_t length);
uint64_t update_crc(uint64_t crc, std::fstream &file, size_t length);
uint64_t get_crc(char *buffer, size_t length);
uint64_t get_crc(std::fstream &file, size_t length);

								  //  0x89 0x50 0x4E 0x47  0x0D  0x0A    0x1A  0x0A
const char PNG_FILE_SIGNATURE[] = {'\211', 'P', 'N', 'G', '\r', '\n', '\032', '\n'};

struct png_chunk {
	
	png_chunk();
	png_chunk(char *buffer);
	png_chunk(std::fstream &file);
	
	void get_chunk(std::fstream &file);

	std::string to_string() const;
	
	// Number of bytes in the chunk's data field ONLY, does not include header size
	uint32_t data_length{0};
	// Each character is restricted to 'A'-'Z' and 'a'-'z'
	char chunk_type[5]{0};
	// Data bytes, can be 0 length
	char *chunk_data{nullptr};
	uint32_t chunk_crc{0};
	
};

/* Specialized chunk info */

// 3 requires a PLTE chunk (png_palette), 2 and 6 may have one, 0 and 4 cannot have one
enum PNG_COLOR_TYPE : uint8_t {
	GREYSCALE 		= 0,
	TRUECOLOR 		= 2,
	INDEXED_COLOR 	= 3,
	GREYSCALE_ALPHA	= 4,
	TRUECOLOR_ALPHA	= 6
};

// IHDR type, always required
struct png_header {
	
	char file_signature[8]{0};
	uint32_t width{0};
	uint32_t height{0};
	uint8_t bit_depth{0};
	PNG_COLOR_TYPE color_type{PNG_COLOR_TYPE::GREYSCALE};
	uint8_t compression_method{0};
	uint8_t filter_method{0};
	uint8_t interlace_method{0};
	
};

// PLTE, required for 3, optional for 2/6, disallowed for 0/4
struct png_palette {
	
	pixel *palette_entries;
	// 1 + palette_count for the number of entries
	uint8_t palette_count;
	
};

// tRNS, optional for 0/2/3, disallowed for 4/6
struct png_transparency {
	
	// 0: 2 byte grey sample value
	// 2: 2 byte red, 2 byte blue, 2 byte green sample values
	// 3: n bytes, alpha for each associated PLTE chunk entry. If size < palette_count, all further are 0xFF
	uint8_t *sample_data{nullptr};
	uint8_t sample_count{0};
	
};

// cHRM, optional for all 
struct png_colorspace {
	
	uint32_t white_point_x{0};
	uint32_t white_point_y{0};
	uint32_t red_x{0};
	uint32_t red_y{0};
	uint32_t green_x{0};
	uint32_t green_y{0};
	uint32_t blue_x{0};
	uint32_t blue_y{0};
	
};

// gAMA, optional for all?
struct png_gamma {
	uint32_t gamma_intensity{0};
};

// iCCP, optional for all
struct png_icc {
	
	char profile_name[80]{0};
	uint8_t compression_method{0};
	uint8_t *compressed_profile{0};
	size_t profile_size;
	
};

// sBIT, optional for all
struct png_significant_bits {
	uint8_t significant_bits[4];
};

enum RENDERING_INTENT : uint8_t {
	PERCEPTUAL				= 0,
	RELATIVE_COLORIMETRIC	= 1,
	SATURATION				= 2,
	ABSOLUTE_COLORIMETRIC	= 3
};

// sRGB, optional for all
// IF an sRGB chunk is to be written, a gAMA and cHRM chunk should also be written
// gAMA: 45455
// cHRM: 31270, 32900, 64000, 33000, 30000, 60000, 15000, 6000
struct png_srgb {
	RENDERING_INTENT render_intent{RENDERING_INTENT::PERCEPTUAL};
};

// tEXt, iTXt, or zTXt, optional for all
struct png_text {
	
	char keyword[80]{0};
	char *text_string{nullptr};
	size_t text_size{0};
	uint8_t compression_method{0};
	
	
	
};

#endif